// Created by 22126 on 2022/12/6.
//
#include "LoadData.hpp"
#include <numeric>
#include <mpi.h>
#include "TimePrint.hpp"
#include <string>

namespace dtb {


    class SimulationOneDimTraffic {

    public:
        /*!
         *
         * @param gpu_out_idx   输出gpu编号
         * @param gpu_in_idx    输入gpu编号
         * @return  输出gpu到输入gpu之间的p2p流量
         */
        static unsigned long
        sim_traffic_between_two_gpu(const unsigned &gpu_out_idx, const unsigned &gpu_in_idx);  //验证正确，计算指定两个卡

        /*!
         * 计算单个gpu到一个组的流量
         * @param gpu_out_idx 输出gpu编号
         * @param gpu_in_list 输入gpu组编号
         * @return 输出gpu到输入gpu组之间的p2p流量
         */
        static unsigned long
        sim_traffic_between_gpu_group(const unsigned &gpu_out_idx, const std::vector<unsigned> &gpu_in_list);


        /*!
         * 对于每个体素的sample只和本身有关，可以直接计算，以避免采样时重复计算
         */
        void generate_sample_range();

        /*!
         * 计算指定维度流量
         * @param argc  mpi参数
         * @param argv  mpi参数
         */
        virtual void compute_simulation_traffic(int argc, char **argv);

        /*!
         * 根据路由表计算执行参数下的发送map
         * @param send_idx 发送gpu卡编号
         * @param recv_idxs 需要计算的列表lists
         * @return 返回对recv_idxs计算的并包特性
         */
        static std::unique_ptr<std::unordered_map<unsigned, std::vector<unsigned >>>
        get_list_send_by_route_table(const unsigned &send_idx, const std::vector<unsigned> &recv_idxs);

    protected:

        /*!
         * 得到采样次数
         * @param out_pop_idx 输出卡体素编号
         * @param gpu_in_idx 输入卡编号
         * @return 返回采样次数
         */
        static unsigned int get_sample_times(const int &out_pop_idx, const unsigned &gpu_in_idx);


        /*!
         * 对于每一个体素需要模拟的神经元数目
         */
        std::array<unsigned long, POP_NUM> pops_sam_range;


        static std::shared_ptr<LoadData> load_data_ptr; //需要load_data实例加载数据

    public:
        SimulationOneDimTraffic();
    };


    std::shared_ptr<LoadData> SimulationOneDimTraffic::load_data_ptr = LoadData::getLoadDataInstance();


    unsigned long SimulationOneDimTraffic::sim_traffic_between_two_gpu(const unsigned int &gpu_out_idx,
                                                                       const unsigned int &gpu_in_idx) {

        unsigned long traffic_gpu_to_gpu{0};

        unsigned int sample_times{0}, sample_range{0};

        for (auto [k_out, v_out]: load_data_ptr->getMapTable()[gpu_out_idx]) {
            sample_times = get_sample_times(k_out, gpu_in_idx);
            sample_range = static_cast<unsigned int> (NEURON_NUM * load_data_ptr->getSizeTable()[k_out] * v_out);
//            printf("sample_times %d, sample range %d, gpu_in_idx %d\n", sample_times, sample_range, gpu_in_idx);
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

            std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
            if (sample_times != 0) {
                if ((sample_range << 2) < sample_times) {
                    traffic_gpu_to_gpu += sample_range;
                } else {
                    traffic_gpu_to_gpu += sample(sample_range, sample_times);
                }
            }
        }
        return traffic_gpu_to_gpu;
    }


    unsigned long
    SimulationOneDimTraffic::sim_traffic_between_gpu_group(const unsigned &gpu_out_idx,
                                                           const std::vector<unsigned> &gpu_in_list) {

        unsigned long traffic_gpu_to_group{0};

        for (auto [k_out, v_out]: load_data_ptr->getMapTable()[gpu_out_idx]) {
            unsigned sample_times{0}, sample_range{0};
            for (auto const &gpu_in_idx: gpu_in_list) {
                sample_times += get_sample_times(k_out,
                                                 gpu_in_idx);     //这里每次get_sample_times都做了double到int的转换，有可能损失较大一些
            }
            sample_range = static_cast<unsigned int> (NEURON_NUM * load_data_ptr->getSizeTable()[k_out] * v_out);

            if (sample_times != 0) {
                if ((sample_range << 2) < sample_times) {
                    traffic_gpu_to_group += sample_range;
                } else {
                    traffic_gpu_to_group += sample(sample_range, sample_times);
                }
            }
//            std::cout << sample_range << " " << sample_times << " " << traffic_gpu_to_gpu<<std::endl;
        }
        return traffic_gpu_to_group;
    }

    unsigned int SimulationOneDimTraffic::get_sample_times(const int &out_pop_idx, const unsigned &gpu_in_idx) {
        double conn_number_estimate{0.0};
        unsigned long long key_temp{0};
        std::vector<double> traffic_src_to_dst;
        for (auto &[k_in, v_in]: load_data_ptr->getMapTable()[gpu_in_idx]) {

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


    SimulationOneDimTraffic::SimulationOneDimTraffic() {
        generate_sample_range();
    }

    void SimulationOneDimTraffic::generate_sample_range() {
        for (decltype(pops_sam_range.size()) i = 0; i != pops_sam_range.size(); ++i) {
            // 因为目前存储的sample_range都是二十万以内，所以用unsigned long存储，后序若扩大则需要注意
            pops_sam_range[i] = static_cast<unsigned long>(NEURON_NUM * load_data_ptr->getSizeTable()[i]);
        }
    }

    void SimulationOneDimTraffic::compute_simulation_traffic(int argc, char **argv) {  //可以精确到每张卡到每张卡的发送

        MPI_Init(&argc, &argv);

        int rank, num;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num);
        if (rank == (num - 1)) {  //num-1号负责计算末尾部分与合并所有数据
            MPI_Status st;
            assert(num <= static_cast<int>(GPU_NUM) && num > 1);   //进程的数目要小于等于模拟的卡数
            unsigned int cal_row_pre_process = GPU_NUM / (num - 1);
            unsigned int len_pre_process = cal_row_pre_process * GPU_NUM;
            std::vector<unsigned int, std::allocator<unsigned int> > traffic_res(GPU_NUM * GPU_NUM);
            for (int i = 0; i < num - 1; ++i) {
                MPI_Recv(&traffic_res[i * cal_row_pre_process], static_cast<int>  (len_pre_process), MPI_UNSIGNED, i,
                         MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            }
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到
            for (auto i = cal_row_pre_process * (num - 1); i != GPU_NUM; ++i) {     //计算最后一部分剩余流量
                auto offset = (i - cal_row_pre_process * (num - 1)) * GPU_NUM;
                for (unsigned j = 0; j < GPU_NUM; ++j) {
                    traffic_res[offset + j] = this->sim_traffic_between_two_gpu(i, j);
                }
            }
            std::string traffic_path = BaseInfo::getInstance()->traffic_tables_path + "/" + "traffic.txt";


            //todo: 为什么vector类型不能写入
            write_vector_data_file(traffic_res, traffic_path);
        } else {   //辅进程负责计算前面各项
            TimePrint t;    //获取计算时间
            unsigned cal_row_pre_process = GPU_NUM / (num - 1);
            unsigned start_gpu_idx = cal_row_pre_process * rank;
            unsigned end_gpu_idx = start_gpu_idx + cal_row_pre_process;
            std::vector<unsigned int> traffic_result(GPU_NUM * (end_gpu_idx
                                                                - start_gpu_idx), 0);
            for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                auto offset = (i - start_gpu_idx) * GPU_NUM;
                for (unsigned j = 0; j < GPU_NUM; ++j) {
                    traffic_result[offset + j] = this->sim_traffic_between_two_gpu(i, j);
                }
            }
            MPI_Send(&traffic_result[0], static_cast<int>(traffic_result.size()), MPI_UNSIGNED, num - 1, 0,
                     MPI_COMM_WORLD);
            printf("process %d ", rank);
            t.print_time();
            MPI_Barrier(MPI_COMM_WORLD);        //阻塞等待全部收到,注意子线程也要加上同步
        }
    }

    std::unique_ptr<std::unordered_map<unsigned, std::vector<unsigned >>>
    SimulationOneDimTraffic::get_list_send_by_route_table(const unsigned int &send_idx,
                                                          const std::vector<unsigned int> &recv_idxs) {


        std::unordered_map<unsigned, std::vector<unsigned >> send_dict;   //用于计算send_idx的最终转发方向
        for (const unsigned &recv_idx: recv_idxs) { //遍历所有需要计算的进程
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

        std::unique_ptr<std::unordered_map<unsigned, std::vector<unsigned >>> send_dict_ptr = std::make_unique<std::unordered_map<unsigned, std::vector<unsigned >>>(
                std::unordered_map<unsigned, std::vector<unsigned >>());


        std::copy_if(send_dict.begin(), send_dict.end(), std::inserter(*send_dict_ptr, (*send_dict_ptr).end()),
                     [send_idx](decltype(send_dict)::value_type const &kv_pair) {
                         return !(kv_pair.second.size() != 1 &&
                                  std::find(kv_pair.second.begin(), kv_pair.second.end(), send_idx) !=
                                  kv_pair.second.end());
                     });
        return send_dict_ptr;
    }

}






















