
#include "SimulationOneDimTraffic.hpp"

namespace dtb {

    class SimulationHighDimTraffic : public SimulationOneDimTraffic {

    private:
        static constexpr short dimensions = 2;
    public:
        /*!
         * 计算指定维度的输入、输出流量(递归版本)
         * @tparam size 流量数据的维度大小
         * @param send_idx 发送gpu卡编号
         * @param stage 发送的阶段
         * @param output_input_traffic 输入与输出流量存储矩阵
         * @param forward_list_send 发送方map
         */
        void simulate_specific_dim_input_output_traffic_per_gpu(const unsigned &send_idx, const unsigned &stage,
                                                                std::array<unsigned, GPU_NUM * 2 *
                                                                                     dimensions> &output_input_traffic,
                                                                const std::unordered_map<unsigned, std::vector<unsigned >> &forward_list_send);

        /*!
        * 计算指定维度流量
        * @param argc  mpi参数
        * @param argv  mpi参数
        */
        void compute_simulation_traffic(int argc, char **argv) override;

        SimulationHighDimTraffic() = default;
    };


    void SimulationHighDimTraffic::simulate_specific_dim_input_output_traffic_per_gpu(const unsigned int &send_idx,
                                                                                      const unsigned int &stage,
                                                                                      std::array<unsigned, GPU_NUM * 2 *
                                                                                                           dimensions> &output_input_traffic,
                                                                                      const std::unordered_map<unsigned int, std::vector<unsigned int>> &forward_list_send) {
        if (stage < 0) {
            return;
        }
        for (auto &in_idx_pair: forward_list_send) {
            if (in_idx_pair.second.size() == 1) {
                if (!is_in_same_node(send_idx, in_idx_pair.second[0])) {
                    auto temp_traffic = sim_traffic_between_two_gpu(send_idx, in_idx_pair.first);

                    output_input_traffic[send_idx * dimensions + stage] += temp_traffic;
                    output_input_traffic[in_idx_pair.second[0] * dimensions + stage + dimensions] += temp_traffic;
                    return;
                } else {
                    auto temp_traffic = sim_traffic_between_gpu_group(send_idx, in_idx_pair.second);

                    output_input_traffic[send_idx * dimensions + stage] += temp_traffic;
                    output_input_traffic[in_idx_pair.first * dimensions + stage + dimensions] += temp_traffic;
                    auto const &forward_sub_idx = get_list_send_by_route_table(in_idx_pair.first,
                                                                               in_idx_pair.second);

                    simulate_specific_dim_input_output_traffic_per_gpu(in_idx_pair.first, stage - 1,
                                                                       output_input_traffic, *forward_sub_idx);
                }
            }

        }
    }

    void SimulationHighDimTraffic::compute_simulation_traffic(int argc, char **argv) {
        MPI_Init(&argc, &argv);

        int rank, num;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num);
        if (rank == (num - 1)) {  //num-1号负责计算末尾部分与合并所有数据
            MPI_Status st;
            assert(num <= static_cast<int>(GPU_NUM) && num > 1);   //进程的数目要小于等于模拟的卡数
            unsigned int cal_row_pre_process = GPU_NUM / (num - 1);
            std::array<unsigned int, GPU_NUM * 2 * dimensions> output_input_traffic{};
            for (int i = 0; i < num - 1; ++i) {
                std::array<unsigned int, GPU_NUM * 2 * dimensions> temp_traffic{};
                MPI_Recv(&temp_traffic[0], static_cast<int>  (GPU_NUM * 2 * dimensions), MPI_UNSIGNED, i,
                         MPI_ANY_TAG, MPI_COMM_WORLD, &st);


                std::transform(output_input_traffic.begin(), output_input_traffic.end(),
                               temp_traffic.begin(), output_input_traffic.begin(),
                               [](unsigned a, unsigned b) { return a + b; });
            }
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到

            std::vector<unsigned> recv_lists(GPU_NUM, 0);
            std::generate(recv_lists.begin(), recv_lists.end(), [i = 1]()mutable { return i++; });
            for (auto i = cal_row_pre_process * (num - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量
                auto const &forward_idx = get_list_send_by_route_table(i, recv_lists);
                simulate_specific_dim_input_output_traffic_per_gpu(i, dimensions - 1, output_input_traffic,
                                                                   *forward_idx);
            }
            std::string traffic_path =
                    BaseInfo::getInstance()->traffic_tables_path + "/" + "traffic_" + std::to_string(dimensions) +
                    "_dim.txt";
            write_vector_data_file(output_input_traffic, traffic_path);
        } else {   //辅进程负责计算前面各项
            TimePrint t;    //获取计算时间
            unsigned cal_row_pre_process = GPU_NUM / (num - 1);
            unsigned start_gpu_idx = cal_row_pre_process * rank;
            unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;

            // 这里是否需要初始化
            std::array<unsigned int, GPU_NUM * 2 * dimensions> output_input_traffic{};

            std::vector<unsigned> recv_lists(GPU_NUM, 0);
            std::generate(recv_lists.begin(), recv_lists.end(), [i = 1]()mutable { return i++; });
            for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                auto const &forward_idx = get_list_send_by_route_table(i, recv_lists);
                simulate_specific_dim_input_output_traffic_per_gpu(i, dimensions - 1, output_input_traffic,
                                                                   *forward_idx);
            }

            MPI_Send(&output_input_traffic[0], static_cast<int>(GPU_NUM * 2 * dimensions), MPI_UNSIGNED, num - 1, 0,
                     MPI_COMM_WORLD);
            printf("process %d ", rank);
            t.print_time();
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
        }


    }


}