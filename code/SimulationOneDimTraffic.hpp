// Created by 22126 on 2022/12/6.
//
#pragma once

#include <numeric>
#include <mpi.h>
#include "SimulationTrafficUtils.hpp"
#include "MpiManage.hpp"
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


        SimulationOneDimTraffic(int argc, char **argv);

    private:
        std::string traffic_path = BaseInfo::getInstance()->traffic_tables_path + "/" + "traffic.txt";

    };


    void SimulationOneDimTraffic::compute_simulation_traffic() {  //可以精确到每张卡到每张卡的发送
        if (pro_rank == master_rank) {  //num-1号负责计算末尾部分与合并所有数据
            MPI_Status st;
            assert(pro_size <= static_cast<int>(GPU_NUM) && pro_size > 1);   //进程的数目要小于等于模拟的卡数
            unsigned int cal_row_pre_process = GPU_NUM / (pro_size - 1);
            unsigned int len_pre_process = cal_row_pre_process * GPU_NUM;
            std::vector<unsigned int, std::allocator<unsigned int> > traffic_res(GPU_NUM * GPU_NUM);
            for (int i = 0; i < pro_size - 1; ++i) {
                MPI_Recv(&traffic_res[i * cal_row_pre_process], static_cast<int>  (len_pre_process), MPI_UNSIGNED, i,
                         MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            }
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到
            for (auto i = cal_row_pre_process * (pro_size - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量
                std::cout << i << std::endl;
                auto offset = (i - cal_row_pre_process * (pro_size - 1)) * GPU_NUM;
                for (unsigned j = 0; j < GPU_NUM; ++j) {
                    traffic_res[offset + j] = this->sim_traffic_between_two_gpu(i, j);
                }
            }

            //array类型不能定义，因为这里含有GPU_NUM*GPU_NUM个元素，因为array含有的元素个数是固定的，因此其会将空间直接开辟在栈上
            // 栈内存当定义数组元素过多时就会溢出，如果想要使用array可以用智能指针管理一个定长array,这样就可以把array开辟在堆上
            write_vector_data_file(traffic_res, traffic_path, GPU_NUM);
        } else {   //辅进程负责计算前面各项
            TimePrint t;    //获取计算时间
            unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
            unsigned start_gpu_idx = cal_row_pre_process * pro_rank;
            unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;
            std::vector<unsigned int> traffic_result(GPU_NUM * (end_gpu_idx
                                                                - start_gpu_idx), 0);
            for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                auto offset = (i - start_gpu_idx) * GPU_NUM;
                for (unsigned j = 0; j < GPU_NUM; ++j) {
                    traffic_result[offset + j] = this->sim_traffic_between_two_gpu(i, j);
                }
            }
            MPI_Send(&traffic_result[0], static_cast<int>(traffic_result.size()), MPI_UNSIGNED, master_rank, 0,
                     MPI_COMM_WORLD);
            printf("process %d ", pro_rank);
            t.print_time();
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
        }
    }

    SimulationOneDimTraffic::SimulationOneDimTraffic(int argc, char **argv) : MpiManage(argc, argv) {}


}






















