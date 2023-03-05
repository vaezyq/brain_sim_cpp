/*******************************************************************************
 * @file
 * @brief  计算一维路由流量
 * @details
 * @author
 * @date       2022-12-06
 * @version    V1.0
 *******************************************************************************/


#pragma once

#include <numeric>
#include "traffic/utils/SimulationTrafficUtils.hpp"
#include "mpi/MpiManage.hpp"
#include <string>

namespace dtb {

    //只用于实现并行计算全部流量，因为是并行程序，所以只有并行调用部分，剥离出来SimulationTrafficUtils类主要便于串行测试
    class SimulationOneDimTraffic : public SimulationTrafficUtils, public MpiManage {

    public:
        /*!
         * 计算指定维度流量
         * @param argc  mpi参数
         * @param argv  mpi参数
         */
        virtual void compute_simulation_traffic();

        /*!
         * 对mpi send通信的一个测试，用于测试集群MPI是否正常
         */
        void mpi_comm_test();

        /*！
         * 得到写入流量文件名
         */
        virtual std::string get_write_traffic_file_name();


        SimulationOneDimTraffic(int argc, char **argv);

    };


    void SimulationOneDimTraffic::compute_simulation_traffic() {  //可以精确到每张卡到每张卡的发送
        if (pro_rank == master_rank) {  //num-1号负责计算末尾部分与合并所有数据
            MPI_Status st;
            assert(pro_size <= (static_cast<int>(GPU_NUM) - 1) && pro_size > 1);   //进程的数目要小于等于模拟的卡数
            unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
            unsigned len_pre_process = cal_row_pre_process * GPU_NUM;
            std::vector<traffic_size_type, std::allocator<traffic_size_type> > traffic_res(GPU_NUM * GPU_NUM);
            for (int i = 0; i < master_rank; ++i) {
                MPI_Recv(&traffic_res[i * len_pre_process], static_cast<int>  (len_pre_process), MPI_DOUBLE, i,
                         MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            }
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到
            for (auto i = cal_row_pre_process * (pro_size - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量
                std::cout << i << "*" << std::endl;
                auto offset = i * GPU_NUM;
                for (unsigned j = 0; j < GPU_NUM; ++j) {
                    traffic_res[offset + j] = this->sim_traffic_between_two_gpu(i, j);
                }
            }

//            for (int i = 0; i < GPU_NUM; ++i) {
//                std::cout << traffic_res[i] << " ";
//            }
            //array类型不能定义，因为这里含有GPU_NUM*GPU_NUM个元素，因为array含有的元素个数是固定的，因此其会将空间直接开辟在栈上
            // 栈内存当定义数组元素过多时就会溢出，如果想要使用array可以用智能指针管理一个定长array,这样就可以把array开辟在堆上
            std::string traffic_file_write_path =
                    BaseInfo::getInstance()->traffic_read_write_path + get_write_traffic_file_name();


            save_one_dim_data_to_binary_file(traffic_file_write_path,traffic_res);

            std::cout << "simulation traffic finished" << std::endl;
            std::cout << traffic_file_write_path << " saved." << std::endl;
        } else {   //辅进程负责计算前面各项
            TimePrint t;    //获取计算时间
            unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
            unsigned start_gpu_idx = cal_row_pre_process * pro_rank;
            unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;
            std::vector<traffic_size_type> traffic_result(GPU_NUM * (end_gpu_idx
                                                                     - start_gpu_idx), 0);
            for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                auto offset = (i - start_gpu_idx) * GPU_NUM;
                for (unsigned j = 0; j < GPU_NUM; ++j) {
                    traffic_result[offset + j] = this->sim_traffic_between_two_gpu(i, j);
                }
            }
            MPI_Send(&traffic_result[0], static_cast<int>(traffic_result.size()), MPI_DOUBLE, master_rank, 0,
                     MPI_COMM_WORLD);
            printf("process %d ", pro_rank);
            t.print_time();
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
        }
    }

    SimulationOneDimTraffic::SimulationOneDimTraffic(int argc, char **argv) : MpiManage(argc, argv) {}

    void SimulationOneDimTraffic::mpi_comm_test() {
        if (pro_rank == master_rank) {
            MPI_Status st;
            std::vector<int> rank_num(pro_size, 0);
            for (int i = 0; i < master_rank; ++i) {
                MPI_Recv(&rank_num[i], 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
                std::cout << "received " << rank_num[i] << " from rank: " << i << std::endl;
            }
            MPI_Barrier(MPI_COMM_WORLD);
            rank_num[master_rank] = master_rank;
            std::cout << "communication finished" << std::endl;
            std::cout << "rank num accumulate: " << std::accumulate(rank_num.begin(), rank_num.end(), 0) << std::endl;
        } else {
            int num = pro_rank;
            MPI_Send(&num, 1, MPI_INT, master_rank, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    std::string SimulationOneDimTraffic::get_write_traffic_file_name() {

        auto base_info_ptr = BaseInfo::getInstance();
        std::string map_traffic = base_info_ptr->map_file_name.substr(0, base_info_ptr->map_file_name.size() - 4);
        return "one_dim_traffic_" + map_traffic + ".txt";
    }


}






















