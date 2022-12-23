//
// Created by 22126 on 2022/12/11.
//
#pragma once

#include "MapSplitMethod.hpp"
#include "SimulationTrafficUtils.hpp"
#include "MpiManage.hpp"
#include "MapSplitMethod.hpp"
#include "SimTrafficForGenMapUtils.hpp"


namespace dtb {
    class GenerateMapParallel : public SimulationTrafficUtils, public MpiManage, public SimTrafficForGenMapUtils {
    public:
        GenerateMapParallel(int argc, char **argv, Iter_Criteria iterCriteria = Iter_Criteria::OUTPUT_INPUT_MAX);

        void generate_map_by_balance_traffic();

//        void generate_map_by_balance_traffic_test();

    private:

        unsigned iter_numbers = 0;  //迭代步数

        inline bool stop_iter_by_step() {
            return iter_numbers++ < 4;
        }

    };

    GenerateMapParallel::GenerateMapParallel(int argc, char **argv, Iter_Criteria iterCriteria) : MpiManage(argc, argv),
                                                                                                  SimTrafficForGenMapUtils(

                                                                                                          iterCriteria) {}


    void GenerateMapParallel::generate_map_by_balance_traffic() {

        if (pro_rank == master_rank) {        //主进程负责拆分过程
            auto base_info_p = BaseInfo::getInstance();
            load_traffic_for_gen_map(output_traffic, input_traffic,
                                     base_info_p->traffic_read_write_path + "/" + base_info_p->traffic_file_name,
                                     dimensions);

            std::array<traffic_size_type, GPU_NUM> criteria_traffic{};
            while (stop_iter_by_step()) {
                std::cout << "start the " << iter_numbers << "th iteration" << std::endl;
                get_criteria_traffic_by_output_input_traffic(criteria_traffic);
                print_max_min_aver_traffic_info(criteria_traffic);
                // 发送用于拆分的流量
                MPI_Bcast(&criteria_traffic[0], GPU_NUM, MPI_DOUBLE, master_rank, MPI_COMM_WORLD);
                std::cout << "send traffic finished" << std::endl;
                MPI_Barrier(MPI_COMM_WORLD);
                msp_ptr->split_pop_by_swap_max_min_pop(criteria_traffic);        //划分流量代码要所有进程都执行,map拆分
                msp_ptr->print_split_result();
                std::string map_file_path =
                        BaseInfo::getInstance()->map_write_path + "/" + std::to_string(iter_numbers) + "_iter.txt";
                write_map_data_to_file(SimTrafficForGenMapUtils::load_data_ptr->getMapTable(), map_file_path);
                //开始计算流量
                MPI_Status st;
                assert(pro_size <= (static_cast<int>(GPU_NUM) - 1) && pro_size > 1);   //进程的数目要小于等于模拟的卡数
                unsigned int cal_row_pre_process = GPU_NUM / (pro_size - 1);


                constexpr unsigned long array_len = (GPU_NUM) << 1;   //分别是输入流量和输出流量
                //todo: 这里是否需要更换为unsigned long
                for (int i = 0; i < master_rank; ++i) {
                    std::array<traffic_size_type, array_len> temp_traffic{};
                    MPI_Recv(&temp_traffic[0], GPU_NUM, MPI_DOUBLE, i,
                             0, MPI_COMM_WORLD, &st);       //接收输出流量


                    MPI_Recv(&temp_traffic[GPU_NUM], GPU_NUM, MPI_DOUBLE, i,
                             1, MPI_COMM_WORLD, &st);      //接收输入流量

//                    std::cout << "output intput traffic from " << i << std::endl;
//                    std::for_each(temp_traffic.begin(), temp_traffic.end(), [](int i) { std::cout << i << " "; });
//                    std::cout << std::endl;
                    std::transform(output_traffic.begin(), output_traffic.end(),
                                   temp_traffic.begin(), output_traffic.begin(),
                                   [](auto &a, auto &b) { return a + b; });

                    std::transform(input_traffic.begin(), input_traffic.end(),
                                   temp_traffic.begin() + GPU_NUM, input_traffic.begin(),
                                   [](auto &a, auto &b) { return a + b; });

                    auto temp_map_table = msp_ptr->getMapTableBeforeChange();
                    SimulationTrafficUtils::load_data_ptr->getMapTable() = std::move(temp_map_table);
                }
                MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到
                for (auto i = cal_row_pre_process * (pro_size - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量
                    compute_2_dim_output_input_traffic_for_map_iter_no_recursive(i);
                }
                std::cout << "calculate traffic finished" << std::endl;
            }
        } else {
            std::array<traffic_size_type, GPU_NUM> criteria_traffic{};
            while (stop_iter_by_step()) {
                MPI_Bcast(&criteria_traffic[0], GPU_NUM, MPI_DOUBLE, master_rank, MPI_COMM_WORLD);
                MPI_Barrier(MPI_COMM_WORLD);
                msp_ptr->split_pop_by_swap_max_min_pop(criteria_traffic);        //划分流量代码要所有进程都执行,map拆分
                //开始计算流量
                TimePrint t;    //获取计算时间
                unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
                unsigned start_gpu_idx = cal_row_pre_process * pro_rank;
                unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;

                output_traffic_temp.fill(0);   //清空元素
                input_traffic_temp.fill(0);     //清空元素
//                std::cout << "changed gpu idx: ";
//                std::for_each(msp_ptr->getChangedGpuIdx().begin(), msp_ptr->getChangedGpuIdx().end(),
//                              [](int i) { std::cout << i << " "; });
                for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                    compute_2_dim_output_input_traffic_for_map_iter_no_recursive(i);
                }
                if (pro_rank % 500 == 0) {
                    std::cout << pro_rank << " ";
                    t.print_time();
                }
                MPI_Send(&output_traffic_temp[0], GPU_NUM, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD);
                MPI_Send(&input_traffic_temp[0], GPU_NUM, MPI_DOUBLE, master_rank, 1, MPI_COMM_WORLD);
                MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
                auto temp_map_table = msp_ptr->getMapTableBeforeChange();
                SimulationTrafficUtils::load_data_ptr->getMapTable() = std::move(temp_map_table);
            }
        }


    }

//    void GenerateMapParallel::generate_map_by_balance_traffic_test() {
//        if (pro_rank == master_rank) {        //主进程负责拆分过程
//            std::array<unsigned long, GPU_NUM> criteria_traffic{};
//
//            while (stop_iter_by_step()) {
//                if (iter_numbers == 1) {   //第一次迭代
//                    auto base_info_p = BaseInfo::getInstance();
//                    load_traffic_for_gen_map(output_traffic, input_traffic,
//                                             base_info_p->traffic_tables_path + "/" + base_info_p->traffic_file_name,
//                                             dimensions);
//                }
//                std::cout << "start the " << iter_numbers << "th iteration" << std::endl;
//                get_criteria_traffic_by_output_input_traffic(criteria_traffic);
//                print_max_min_aver_traffic_info(criteria_traffic);
//                // 发送用于拆分的流量
//                MPI_Bcast(&criteria_traffic[0], GPU_NUM, MPI_UNSIGNED_LONG, master_rank, MPI_COMM_WORLD);
//                std::cout << "send traffic finished" << std::endl;
//                MPI_Barrier(MPI_COMM_WORLD);
//                mp.split_pop_by_swap_max_min_pop(criteria_traffic);        //划分流量代码要所有进程都执行,map拆分
//                mp.print_split_result();
//                std::string map_file_path =
//                        BaseInfo::getInstance()->map_path + "/" + std::to_string(iter_numbers) + "_iter.txt";
//                write_map_data_to_file(load_data_ptr->getMapTable(), map_file_path);
//                //开始计算流量
//                MPI_Status st;
//                assert(pro_size <= (static_cast<int>(GPU_NUM) - 1) && pro_size > 1);   //进程的数目要小于等于模拟的卡数
//                unsigned int cal_row_pre_process = GPU_NUM / (pro_size - 1);
//                constexpr unsigned long array_len = (GPU_NUM * dimensions) << 1;
//                //todo: 这里是否需要更换为unsigned long
//                std::array<unsigned int, array_len> output_input_traffic{};
//                for (int i = 0; i < master_rank; ++i) {
//                    std::array<unsigned int, array_len> temp_traffic{};
//                    MPI_Recv(&temp_traffic[0], static_cast<int>  (GPU_NUM * 2 * dimensions), MPI_UNSIGNED, i,
//                             MPI_ANY_TAG, MPI_COMM_WORLD, &st);
//                    std::transform(output_input_traffic.begin(), output_input_traffic.end(),
//                                   temp_traffic.begin(), output_input_traffic.begin(),
//                                   [](unsigned a, unsigned b) { return a + b; });
//                }
//                MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到
//                std::vector<unsigned> recv_lists(GPU_NUM, 0);
//                std::generate(recv_lists.begin(), recv_lists.end(), [i = 1]()mutable { return i++; });
//                for (auto i = cal_row_pre_process * (pro_size - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量
//                    simulate_2_dim_input_output_traffic_per_gpu_no_recursive(i, output_input_traffic);
//                }
//                std::cout << "calculate traffic finished" << std::endl;
//                auto dou_dim = dimensions << 1;
//                auto start = output_input_traffic.begin() - dou_dim;
//                auto middle = start + dimensions;
//                auto end = middle + dimensions;
//                for (unsigned long i = 0; i < GPU_NUM; ++i) {
//                    std::advance(start, dou_dim);
//                    std::advance(middle, dou_dim);
//                    std::advance(end, dou_dim);
//                    output_traffic[i] = std::accumulate(start, middle, 0UL);
//                    input_traffic[i] = std::accumulate(middle, end, 0UL);
//                }
//            }
//        } else {
//            std::array<unsigned long, GPU_NUM> criteria_traffic{};
//            while (stop_iter_by_step()) {
//                MPI_Bcast(&criteria_traffic[0], GPU_NUM, MPI_UNSIGNED_LONG, master_rank, MPI_COMM_WORLD);
//                MPI_Barrier(MPI_COMM_WORLD);
//                mp.split_pop_by_swap_max_min_pop(criteria_traffic);        //划分流量代码要所有进程都执行,map拆分
//                //开始计算流量
//                TimePrint t;    //获取计算时间
//                unsigned cal_row_pre_process = GPU_NUM / (pro_size - 1);
//                unsigned start_gpu_idx = cal_row_pre_process * pro_rank;
//                unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;
//
//                std::array<unsigned int, GPU_NUM * 2 * dimensions> output_input_traffic{};
//                for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
//                    simulate_2_dim_input_output_traffic_per_gpu_no_recursive(i, output_input_traffic);
//                }
//                t.print_time();
//                MPI_Send(&output_input_traffic[0], static_cast<int>(GPU_NUM * 2 * dimensions), MPI_UNSIGNED,
//                         master_rank, 0,
//                         MPI_COMM_WORLD);
//                MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
//            }
//        }
//    }




};

