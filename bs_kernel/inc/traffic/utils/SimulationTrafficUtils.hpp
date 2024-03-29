/*******************************************************************************
 * @file
 * @brief  用于计算卡流量的工具类，便于计算
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/


#pragma once

#include <numeric>

#include "../../utils/TimePrint.hpp"
#include <string>
#include "../../data/LoadData.hpp"
#include <thread>
#include <mutex>


namespace dtb {

//模拟流量的工具函数类，和并行计算流量代码分开，便于调试
    class SimulationTrafficUtils {
    public:

        /*!
            *
            * @param gpu_out_idx   输出gpu编号
            * @param gpu_in_idx    输入gpu编号
            * @return  输出gpu到输入gpu之间的p2p流量
            */
        static traffic_size_type
        sim_traffic_between_two_gpu(const gpu_size_type &gpu_out_idx, const gpu_size_type &gpu_in_idx,
                                    const std::vector<std::unordered_map<gpu_size_type, double> > &map_table = load_data_ptr->getMapTable());  //验证正确，计算指定两个卡

        /*!
         * 计算单个gpu到一个组的流量
         * @param gpu_out_idx 输出gpu编号
         * @param gpu_in_list 输入gpu组编号
         * @return 输出gpu到输入gpu组之间的p2p流量
         */
        static traffic_size_type
        sim_traffic_between_gpu_group(const gpu_size_type &gpu_out_idx, const std::vector<gpu_size_type> &gpu_in_list,
                                      const std::vector<std::unordered_map<gpu_size_type, double> > &map_table = load_data_ptr->getMapTable());


        /*!
         * 对于每个体素的sample只和本身有关，可以直接计算，以避免采样时重复计算
         */
        void generate_sample_range();

        /*!
         * 根据路由表计算执行参数下的发送map
         * @param send_idx 发送gpu卡编号
         * @param recv_idxs 需要计算的列表lists
         * @return 返回对recv_idxs计算的并包特性
         */
        static std::unique_ptr<std::unordered_map<gpu_size_type, std::vector<gpu_size_type >>>
        get_list_send_by_route_table(const gpu_size_type &send_idx, const std::vector<gpu_size_type> &recv_idxs);


        /*!
        * 计算二维流量的非迭代版本，目前是有效的版本，可以注意到所有维度的发送方都是最初的send_idx
        * //todo: 关于上述的迭代版本和三维四维的流量如何计算还需要进一步验证
        * @param send_idx
        * @param output_input_traffic
        */
        void simulate_2_dim_input_output_traffic_per_gpu_no_recursive(const gpu_size_type &send_idx,
                                                                      std::array<traffic_size_type,
                                                                              GPU_NUM
                                                                                      << 2> &output_input_traffic);

        /*!
         * 计算发生变化的体素编号的变化流量
         * @param send_idx  发送方gpu编号
         * @param changed_pop_lists_in_send  发生变化的体素编号
         * @param recv_idx_lists   接收gpu编号
         * @param map_table  map表
         * @return  返回发生变化的gpu产生的流量
         */
        traffic_size_type
        compute_pop_traffic(const gpu_size_type &send_idx, const std::vector<gpu_size_type> &changed_pop_lists_in_send,
                            const std::vector<gpu_size_type> &recv_idx_lists,
                            std::vector<std::unordered_map<gpu_size_type, double> > &map_table = load_data_ptr->getMapTable());


        /*!
         * 计算流量的线程入口函数
         * @param forward_list_send   转发表
         * @param send_idx  发送卡编号
         * @param output_input_traffic 存储用于计算的流量
         * @param start 需要计算的接收卡起始编号
         * @param end 需要计算的接收卡结束编号
         */
        void calculate_traffic_by_thread_func(
                std::unordered_map<gpu_size_type, std::vector<gpu_size_type >> &forward_list_send,
                const gpu_size_type &send_idx, std::array<traffic_size_type, GPU_NUM << 2> &output_input_traffic,
                std::vector<gpu_size_type>::iterator start, std::vector<gpu_size_type>::iterator end);


        /*!
         * 使用多线程计算卡流量的版本
         * @param send_idx  发送idx
         * @param output_input_traffic  输入输出流量
         */
        void simulate_2_dim_input_output_traffic_per_gpu_no_recursive_thread_version(const gpu_size_type &send_idx,
                                                                                     std::array<traffic_size_type,
                                                                                             GPU_NUM
                                                                                                     << 2> &output_input_traffic);


        SimulationTrafficUtils();


        void simulate_two_dim_input_output_traffic_by_physical_topology(const gpu_size_type &send_idx,
                                                                        std::array<traffic_size_type,
                                                                                GPU_NUM
                                                                                        << 2> &output_input_traffic);


    protected:
        /*!
            * 得到采样次数
            * @param out_pop_idx 输出卡体素编号
            * @param gpu_in_idx 输入卡编号
            * @return 返回采样次数
            */
        static traffic_size_type get_sample_times(const pop_size_type &out_pop_idx, const gpu_size_type &gpu_in_idx,
                                                  const std::vector<std::unordered_map<gpu_size_type, double> > &map_table = load_data_ptr->getMapTable());

        /*!
         * 对于每一个体素需要模拟的神经元数目
         */
        std::array<traffic_size_type, POP_NUM> pops_sam_range;

        static const std::shared_ptr<LoadData> load_data_ptr;//需要load_data实例加载数据


    private:
        std::mutex traffic_table_mutex;         //写流量的多线程锁
    };

    const std::shared_ptr<LoadData>   SimulationTrafficUtils::load_data_ptr = LoadData::getLoadDataInstance();


//
//    std::shared_ptr<LoadData> const load_data_ptr =

    traffic_size_type SimulationTrafficUtils::sim_traffic_between_two_gpu(const gpu_size_type &gpu_out_idx,
                                                                          const gpu_size_type &gpu_in_idx,
                                                                          const std::vector<std::unordered_map<gpu_size_type, double> > &map_table) {
        traffic_size_type traffic_gpu_to_gpu{0};

        traffic_size_type sample_times{0}, sample_range{0};

        for (auto [k_out, v_out]: map_table[gpu_out_idx]) {
            sample_times = get_sample_times(k_out, gpu_in_idx, map_table);
            sample_range = NEURON_NUM * load_data_ptr->getSizeTable()[k_out] * v_out;
//            printf("sample_times %lf, sample range %lf, gpu_in_idx %d\n", sample_times, sample_range, gpu_in_idx);
//            if (sample_times != 0) {
//                printf("sample_times %d, sample range %d, gpu_in_idx %d gpu_out_idx %d\n", sample_times, sample_range,
//                       gpu_in_idx, gpu_out_idx);
////                printf("gpu_in_idx %d\n")
////                sample_range = static_cast<unsigned int> (pops_sam_range[k_out] * v_out);
//            }

//            if (sample_times != 0) {
//                printf("sample_times %d, sample range %d\n", sample_times, sample_range);
////                printf("gpu_in_idx %d\n")
////                sample_range = static_cast<unsigned int> (pops_sam_range[k_out] * v_out);
//            }


            if (sample_times != 0) {
                if ((sample_range * 2) < sample_times) {
                    traffic_gpu_to_gpu += sample_range;
                } else {
                    traffic_gpu_to_gpu += sample(sample_range, sample_times);
                }
            }
        }
        return traffic_gpu_to_gpu;
    }

    traffic_size_type SimulationTrafficUtils::sim_traffic_between_gpu_group(const gpu_size_type &gpu_out_idx,
                                                                            const std::vector<gpu_size_type> &gpu_in_list,
                                                                            const std::vector<std::unordered_map<gpu_size_type, double> > &map_table) {

        traffic_size_type traffic_gpu_to_group{0};

        for (auto [k_out, v_out]: map_table[gpu_out_idx]) {
            traffic_size_type sample_times{0}, sample_range{0};
            for (auto const &gpu_in_idx: gpu_in_list) {
                sample_times += get_sample_times(k_out,
                                                 gpu_in_idx,
                                                 map_table);     //这里每次get_sample_times都做了double到int的转换，有可能损失较大一些
            }
            sample_range = static_cast<unsigned int> (NEURON_NUM * load_data_ptr->getSizeTable()[k_out] * v_out);

            if (sample_times != 0) {
                if ((sample_range * 2) < sample_times) {
                    traffic_gpu_to_group += sample_range;
                } else {
                    traffic_gpu_to_group += sample(sample_range, sample_times);
                }
            }
//            std::cout << sample_range << " " << sample_times << " " << traffic_gpu_to_gpu<<std::endl;
        }
        return traffic_gpu_to_group;
    }

    void SimulationTrafficUtils::generate_sample_range() {
        for (decltype(pops_sam_range.size()) i = 0; i != pops_sam_range.size(); ++i) {
            // 因为目前存储的sample_range都是二十万以内，所以用unsigned long存储，后序若扩大则需要注意
            pops_sam_range[i] = static_cast<unsigned long>(NEURON_NUM * load_data_ptr->getSizeTable()[i]);
        }
    }

    traffic_size_type
    SimulationTrafficUtils::get_sample_times(const gpu_size_type &out_pop_idx, const gpu_size_type &gpu_in_idx,
                                             const std::vector<std::unordered_map<gpu_size_type, double> > &map_table) {
        double conn_number_estimate{0.0};
        double key_temp{0};
        std::vector<traffic_size_type> traffic_src_to_dst;
        for (auto &[k_in, v_in]: map_table[gpu_in_idx]) {

            //todo:  这里是unsigned int,如果直接做乘法会出现精度损失，所以这里都要做转型(或许有更快的方法)
            key_temp = static_cast<unsigned long> ( k_in) * (POP_NUM) + (out_pop_idx);
//            std::cout << key_temp << std::endl;
//            std::cout << "out_pop_idx: " << out_pop_idx << " k_in: " << k_in << std::endl;
            if (auto iter = load_data_ptr->getConnDictTable().find(key_temp);iter !=
                                                                             load_data_ptr->getConnDictTable().end()) {
                conn_number_estimate = NEURON_NUM * load_data_ptr->getSizeTable()[k_in] * v_in *
                                       load_data_ptr->getDegreeTable()[k_in] * (iter->second);
                traffic_src_to_dst.emplace_back(conn_number_estimate);
            } else {
                traffic_src_to_dst.emplace_back(0.0);
            }
        }
        return static_cast<unsigned int>(std::accumulate(traffic_src_to_dst.begin(), traffic_src_to_dst.end(), 0.0));
    }

    std::unique_ptr<std::unordered_map<gpu_size_type, std::vector<gpu_size_type >>>
    SimulationTrafficUtils::get_list_send_by_route_table(const gpu_size_type &send_idx,
                                                         const std::vector<gpu_size_type> &recv_idxs) {

        std::unordered_map<gpu_size_type, std::vector<gpu_size_type >> send_dict;   //用于计算send_idx的最终转发方向
        for (const gpu_size_type &recv_idx: recv_idxs) { //遍历所有需要计算的进程
            if (recv_idx == send_idx) {    //如果发送进程和接收进程相等则跳过
                continue;
            } else {
                auto forward_idx = load_data_ptr->getRouteTable()[send_idx][recv_idx];
                if (forward_idx == send_idx &&
                    (!send_dict.count(recv_idx))) {
                    send_dict[recv_idx] = {recv_idx};
                } else {
                    if (!send_dict.count(forward_idx)) {
                        send_dict[forward_idx] = {forward_idx};
                    }
                    send_dict[forward_idx].emplace_back(recv_idx);
                }
            }
        }

        std::unique_ptr<std::unordered_map<gpu_size_type, std::vector<gpu_size_type >>> send_dict_ptr = std::make_unique<std::unordered_map<gpu_size_type, std::vector<gpu_size_type >>>(
                std::unordered_map<gpu_size_type, std::vector<gpu_size_type >>());

        std::copy_if(send_dict.begin(), send_dict.end(), std::inserter(*send_dict_ptr, (*send_dict_ptr).end()),
                     [send_idx](decltype(send_dict)::value_type const &kv_pair) {
                         return !(kv_pair.second.size() != 1 &&
                                  std::find(kv_pair.second.begin(), kv_pair.second.end(), send_idx) !=
                                  kv_pair.second.end());
                     });
        return send_dict_ptr;
    }

    SimulationTrafficUtils::SimulationTrafficUtils() {
        generate_sample_range();
    }

    void SimulationTrafficUtils::simulate_2_dim_input_output_traffic_per_gpu_no_recursive(const gpu_size_type &send_idx,
                                                                                          std::array<traffic_size_type,
                                                                                                  GPU_NUM
                                                                                                          << 2> &output_input_traffic) {

        unsigned long dimensions = 2;
        //计算高维流量的非递归版本，主要用于流量验证
        std::vector<gpu_size_type> recv_lists(dtb::GPU_NUM, 0);
        std::generate(recv_lists.begin(), recv_lists.end(), [i = 0]()mutable { return i++; });

        traffic_size_type temp_traffic{};

        auto const &forward_list_send = get_list_send_by_route_table(send_idx, recv_lists);
        for (auto &in_idx_pair: *forward_list_send) {
            if (in_idx_pair.second.size() == 1) {
                if (!is_in_same_node(send_idx, in_idx_pair.first)) {
                    temp_traffic = sim_traffic_between_two_gpu(send_idx, in_idx_pair.first);
                    output_input_traffic[(send_idx << 2)] += temp_traffic;
                    output_input_traffic[(in_idx_pair.first << 2) + dimensions] += temp_traffic;
                }
            } else {
                temp_traffic = sim_traffic_between_gpu_group(send_idx, in_idx_pair.second);
                output_input_traffic[send_idx << 2] += temp_traffic;
                output_input_traffic[(in_idx_pair.first << 2) + dimensions] += temp_traffic;
                auto forward_sub_idx = get_list_send_by_route_table(in_idx_pair.first,
                                                                    in_idx_pair.second);
                for (auto &in_idx_pair_1: *forward_sub_idx) {
                    if (!is_in_same_node(send_idx, in_idx_pair_1.first)) {
                        temp_traffic = sim_traffic_between_two_gpu(send_idx, in_idx_pair_1.first);
                        output_input_traffic[(in_idx_pair.first << 2) + 1] += temp_traffic;
                        output_input_traffic[(in_idx_pair_1.first << 2) + 1 + dimensions] += temp_traffic;
                    }
                }
            }
        }
    }

    traffic_size_type SimulationTrafficUtils::compute_pop_traffic(const gpu_size_type &send_idx,
                                                                  const std::vector<pop_size_type> &changed_pop_lists_in_send,
                                                                  const std::vector<gpu_size_type> &recv_idx_lists,
                                                                  std::vector<std::unordered_map<gpu_size_type, double>> &map_table) {

        traffic_size_type traffic_gpu_to_group{0};
        for (pop_size_type pop_idx: changed_pop_lists_in_send) {
            traffic_size_type sample_times{0}, sample_range{0};
            for (auto const &gpu_in_idx: recv_idx_lists) {
                sample_times += get_sample_times(pop_idx, gpu_in_idx,
                                                 map_table);     //这里每次get_sample_times都做了double到int的转换，有可能损失较大一些
            }
            sample_range = (NEURON_NUM * load_data_ptr->getSizeTable()[pop_idx] *
                            map_table[send_idx][pop_idx]);
            if (sample_times != 0) {
                if ((sample_range * 2) < sample_times) {
                    traffic_gpu_to_group += sample_range;
                } else {
                    traffic_gpu_to_group += sample(sample_range, sample_times);
                }
            }
        }
        return traffic_gpu_to_group;
    }

    void SimulationTrafficUtils::simulate_2_dim_input_output_traffic_per_gpu_no_recursive_thread_version(
            const gpu_size_type &send_idx, std::array<traffic_size_type, GPU_NUM << 2> &output_input_traffic) {

        std::vector<gpu_size_type> recv_lists(dtb::GPU_NUM, 0);
        std::generate(recv_lists.begin(), recv_lists.end(), [i = 0]()mutable { return i++; });

        auto const &forward_list_send = get_list_send_by_route_table(send_idx, recv_lists);

//        std::map<gpu_size_type, std::vector<gpu_size_type>> send_dict_init;

        std::vector<gpu_size_type> send_key_list;

        std::generate_n(std::back_inserter(send_key_list), forward_list_send->size(),
                        [it = forward_list_send->begin()]()mutable {
                            return (it++)->first;
                        });

//        std::for_each(send_key_list.begin(), send_key_list.end(), [](int i) { std::cout << i << " "; });
//        std::cout << std::endl;

        unsigned long const min_len_per_thread = 2;    //每个线程最少需要处理的数目
        unsigned long const max_threads =
                (send_key_list.size() + min_len_per_thread - 1) / min_len_per_thread;   //最大线程个数
        unsigned long const hardware_threads = std::thread::hardware_concurrency();       //硬件支持并发数目
//        std::cout << "hardware_threads: " << hardware_threads << std::endl;

        unsigned long num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

//        std::cout << "threads num: " << num_threads << std::endl;
        unsigned long const calcu_block_len = send_key_list.size() / num_threads;

//        std::cout << "calcu_block_len: " << calcu_block_len << std::endl;

        std::vector<std::thread> threads(num_threads - 1);
        auto block_start = send_key_list.begin();


        for (auto i = 0; i < (num_threads - 1); ++i) {
            auto block_end = block_start;
            std::advance(block_end, calcu_block_len);

            threads[i] = std::thread(&SimulationTrafficUtils::calculate_traffic_by_thread_func, this,
                                     std::ref(*forward_list_send),
                                     send_idx, std::ref(output_input_traffic), block_start,
                                     block_end);
//            std::mutex iomutex;
//            cpu_set_t cpuset;
//            CPU_ZERO(&cpuset);
//            CPU_SET(i, &cpuset);
//            int rc = pthread_setaffinity_np(threads[i].native_handle(),
//                                            sizeof(cpu_set_t), &cpuset);
//            if (rc != 0) {
//                std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
//            }
//            std::lock_guard<std::mutex> iolock(iomutex);
//            std::cout << "Thread #" << i << ": on CPU " << sched_getcpu() << "\n";

            block_start = block_end;
        }


        //主线程计算最后一部分]
        auto block_end = send_key_list.end();
        calculate_traffic_by_thread_func(*forward_list_send, send_idx, output_input_traffic, block_start, block_end);

        for (auto &thread: threads) {
            thread.join();          //阻塞等待全部线程运行完毕
        }
    }


    void SimulationTrafficUtils::calculate_traffic_by_thread_func(
            std::unordered_map<gpu_size_type, std::vector<gpu_size_type >> &forward_list_send,
            const gpu_size_type &send_idx,
            std::array<traffic_size_type, GPU_NUM << 2> &output_input_traffic,
            std::vector<gpu_size_type>::iterator start, std::vector<gpu_size_type>::iterator end) {

        TimePrint tp;

        std::for_each(start, end, [](gpu_size_type i) { std::cout << i << " "; });
        std::cout << std::endl;

        traffic_size_type temp_traffic{0.0};
        unsigned dimensions = 2;


        for (auto it = start; it != end; ++it) {
//            std::cout << *it << " " << std::this_thread::get_id() << std::endl;
            if (forward_list_send[*it].size() == 1) {
                if (!is_in_same_node(send_idx, *it)) {
                    temp_traffic = sim_traffic_between_two_gpu(send_idx, *it);
                    {
                        std::lock_guard<std::mutex> lg(traffic_table_mutex);         //对本行数据加锁写
                        output_input_traffic[(send_idx << 2)] += temp_traffic;
                        output_input_traffic[(*it << 2) + dimensions] += temp_traffic;
                    }
                }
            } else {
                temp_traffic = sim_traffic_between_gpu_group(send_idx, forward_list_send[*it]);
                {
                    std::lock_guard<std::mutex> lg(traffic_table_mutex);
                    output_input_traffic[send_idx << 2] += temp_traffic;
                    output_input_traffic[(*it << 2) + dimensions] += temp_traffic;
                }
                auto forward_sub_idx = get_list_send_by_route_table(*it,
                                                                    forward_list_send[*it]);
                for (auto &in_idx_pair_1: *forward_sub_idx) {
                    if (!is_in_same_node(send_idx, in_idx_pair_1.first)) {
                        temp_traffic = sim_traffic_between_two_gpu(send_idx, in_idx_pair_1.first);
                        {
                            std::lock_guard<std::mutex> lg(traffic_table_mutex);
                            output_input_traffic[(*it << 2) + 1] += temp_traffic;
                            output_input_traffic[(in_idx_pair_1.first << 2) + 1 + dimensions] += temp_traffic;
                        }

                    }
                }
            }
        }
    }


    void
    SimulationTrafficUtils::simulate_two_dim_input_output_traffic_by_physical_topology(const gpu_size_type &send_idx,
                                                                                       std::array<traffic_size_type,
                                                                                               GPU_NUM
                                                                                                       << 2> &output_input_traffic) {
        unsigned dimensions = 2;

        std::unordered_map<gpu_size_type, std::vector<gpu_size_type >> send_idx_list;
        for (gpu_size_type recv_idx = 0; recv_idx < GPU_NUM; ++recv_idx) {      //得到本次的发送情况
            if (!is_in_same_node(send_idx, recv_idx)) {     //不在同一节点内
                send_idx_list[load_data_ptr->getRouteTable()[send_idx][recv_idx]].emplace_back(recv_idx);
            }
        }

        for (auto &kv: send_idx_list) {
            auto temp_traffic = sim_traffic_between_gpu_group(send_idx, kv.second);
            output_input_traffic[send_idx << 2] += temp_traffic;
            output_input_traffic[(kv.first << 2) + dimensions] += temp_traffic;

            for (auto &e: kv.second) {
                if (!is_in_same_node(e, kv.first)) {    //不在一个节点内

                    temp_traffic = sim_traffic_between_two_gpu(send_idx, e);

                    output_input_traffic[(kv.first << 2) + 1] += temp_traffic;         //第二阶段
                    output_input_traffic[(e << 2) + dimensions + 1] += temp_traffic;
                }
            }
        }
    }


}



