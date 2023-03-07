/*******************************************************************************
 * @file
 * @brief  计算高维路由流量（目前仅能计算二维的）
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/

#pragma once

#include "SimulationOneDimTraffic.hpp"

namespace dtb {

    class SimulationHighDimTraffic : public SimulationOneDimTraffic {

    private:
        static constexpr short dimensions = 2;

        static constexpr short dou_dimensions = dimensions << 1;

        std::string get_write_traffic_file_name() override;

    public:
        /*!
         * 计算指定维度的输入、输出流量(递归版本),目前这段代码存在问题，二维计算应使用非递归版本
         * @tparam size 流量数据的维度大小
         * @param send_idx 发送gpu卡编号
         * @param stage 发送的阶段
         * @param output_input_traffic 输入与输出流量存储矩阵
         * @param forward_list_send 发送方map
         */
        static void
        simulate_specific_dim_input_output_traffic_per_gpu(const gpu_size_type &send_idx, const unsigned &stage,
                                                           std::array<traffic_size_type, GPU_NUM * 2 *
                                                                                         dimensions> &output_input_traffic,
                                                           const std::unordered_map<gpu_size_type, std::vector<gpu_size_type >> &forward_list_send);


        /*!
        * 计算指定维度流量
        * @param argc  mpi参数
        * @param argv  mpi参数
        */
        void compute_simulation_traffic() override;

        SimulationHighDimTraffic(int argc, char **argv);
    };


    void SimulationHighDimTraffic::simulate_specific_dim_input_output_traffic_per_gpu(const gpu_size_type &send_idx,
                                                                                      const gpu_size_type &stage,
                                                                                      std::array<traffic_size_type,
                                                                                              GPU_NUM * 2 *
                                                                                              dimensions> &output_input_traffic,
                                                                                      const std::unordered_map<gpu_size_type, std::vector<gpu_size_type>> &forward_list_send) {
        if (dimensions <= stage) {
            return;
        }
        for (auto &in_idx_pair: forward_list_send) {
            if (in_idx_pair.second.size() == 1) {
                if (!is_in_same_node(send_idx, in_idx_pair.first)) {
                    auto temp_traffic = sim_traffic_between_two_gpu(send_idx, in_idx_pair.first);

                    output_input_traffic[send_idx * dou_dimensions + stage] += temp_traffic;
//                    output_input_traffic[in_idx_pair.first * dou_dimensions + stage + dimensions] += temp_traffic;
                }
            } else {
                auto temp_traffic = sim_traffic_between_gpu_group(send_idx, in_idx_pair.second);
                output_input_traffic[send_idx * dou_dimensions + stage] += temp_traffic;
//                output_input_traffic[in_idx_pair.first * dou_dimensions + stage + dimensions] += temp_traffic;

                auto forward_sub_idx = get_list_send_by_route_table(in_idx_pair.first,
                                                                    in_idx_pair.second);
                simulate_specific_dim_input_output_traffic_per_gpu(in_idx_pair.first, stage + 1,
                                                                   output_input_traffic, *forward_sub_idx);
            }

        }
    }

    void SimulationHighDimTraffic::compute_simulation_traffic() {

        if (pro_rank == master_rank) {  //num-1号负责计算末尾部分与合并所有数据
            MPI_Status st;
            assert(pro_size <= (static_cast<int>(GPU_NUM) - 1) && pro_size > 1);   //进程的数目要小于等于模拟的卡数
            unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
            //todo: 这里是否需要更换为unsigned long
            std::array<traffic_size_type, ((GPU_NUM * dimensions) << 1)> output_input_traffic{};

            for (int i = 0; i < master_rank; ++i) {
                std::array<traffic_size_type, GPU_NUM * 2 * dimensions> temp_traffic{};
                MPI_Recv(&temp_traffic[0], static_cast<int>  (GPU_NUM * 2 * dimensions), MPI_DOUBLE, i,
                         MPI_ANY_TAG, MPI_COMM_WORLD, &st);
                std::transform(output_input_traffic.begin(), output_input_traffic.end(),
                               temp_traffic.begin(), output_input_traffic.begin(),
                               [](auto &a, auto &b) { return a + b; });
            }
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到

            for (auto i = cal_row_pre_process * (pro_size - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量

                simulate_2_dim_input_output_traffic_per_gpu_no_recursive(i, output_input_traffic);

//                auto const forward_idx = get_list_send_by_route_table(i, recv_lists);
//
//                simulate_specific_dim_input_output_traffic_per_gpu(i, 0, output_input_traffic,
//                                                                   *forward_idx);
            }
            std::string write_traffic_path =
                    BaseInfo::getInstance()->traffic_read_write_path + "/" + get_write_traffic_file_name();

            save_one_dim_data_to_binary_file(write_traffic_path,output_input_traffic);


            std::cout << write_traffic_path << " saved." << std::endl;
        } else {   //辅进程负责计算前面各项
            TimePrint t;    //获取计算时间
            unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
            unsigned start_gpu_idx = cal_row_pre_process * pro_rank;
            unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;

            // 这里是否需要初始化
            std::array<traffic_size_type, GPU_NUM * 2 * dimensions> output_input_traffic{};
            std::vector<gpu_size_type> recv_lists(GPU_NUM, 0);
            for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
//                auto const &forward_idx = get_list_send_by_route_table(i, recv_lists);
//                simulate_specific_dim_input_output_traffic_per_gpu(i, dimensions - 1, output_input_traffic,
//                                                                   *forward_idx);
                simulate_2_dim_input_output_traffic_per_gpu_no_recursive_thread_version(i,output_input_traffic);

//                simulate_2_dim_input_output_traffic_per_gpu_no_recursive(i, output_input_traffic);
            }
            MPI_Send(&output_input_traffic[0], static_cast<int>(GPU_NUM * 2 * dimensions), MPI_DOUBLE, master_rank, 0,
                     MPI_COMM_WORLD);
            printf("process %d ", pro_rank);
            t.print_time();
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
        }
    }

    SimulationHighDimTraffic::SimulationHighDimTraffic(int argc, char **argv) : SimulationOneDimTraffic(argc, argv) {
    }

    std::string SimulationHighDimTraffic::get_write_traffic_file_name() {
        auto base_info_ptr = BaseInfo::getInstance();
        std::string map_traffic = base_info_ptr->map_file_name.substr(0, base_info_ptr->map_file_name.size() - 4);

        std::string route_traffic = base_info_ptr->route_file_name.substr(0, base_info_ptr->route_file_name.size() - 4);
        return route_traffic + "_traffic_" + map_traffic + ".txt";
    }

}