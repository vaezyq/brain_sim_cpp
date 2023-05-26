/*******************************************************************************
 * @file
 * @brief  根据物理网络生成路由表类
 * @details
 * @author
 * @date       2023-05-19
 * @version    V1.0
 *******************************************************************************/

#include "../mpi/MpiNodeInfo.hpp"
#include <cassert>
#include <fstream>
#include "../../inc/data/utils/ProcessFileDataUtils.hpp"
#include "../../inc/data/BaseInfo.hpp"

namespace dtb {

    class GenerateRouteByPhysicalTopology : public MpiNodeInfo {

    public:

        void start_generate_physical_topology() {
            start_init_node_info();
            if (pro_rank == master_rank) {
                get_init_physical_group();
                generate_route_by_physics_topology();
                confirm_physical_route_table();

                auto base_info_ptr = dtb::BaseInfo::getInstance();
                string route_table_file_path =
                        base_info_ptr->route_dir_path + "/" + base_info_ptr->route_file_name;

                save_two_dim_data_to_binary_file(route_table_file_path, route_table);

                save_two_dim_route_data_to_txt_file(route_table_file_path + ".txt", route_table,
                                                    {static_cast<unsigned int>(pro_size)});
            }
        }

        GenerateRouteByPhysicalTopology(int argc, char **argv);

        void get_init_physical_group();

        auto find_idx(const unsigned &send_idx);

        void generate_route_by_physics_topology();

        void confirm_physical_route_table() {
            std::cout << "Begin to confirm route table..." << std::endl;
            unsigned max_for_times{};

            for (gpu_size_type src = 0; src < route_table.size(); ++src) {
                for (gpu_size_type dst = 0; dst < route_table.size(); ++dst) {

//                    cout << src << " -> " << dst << "/";
                    gpu_size_type temp_src = src;
                    unsigned count{};
                    while (route_table[temp_src][dst] != temp_src) {

//                        cout << temp_src << " -> " << dst << " by " << route_table[temp_src][dst] << "/";
                        auto it = rank_to_host.find(src);
                        auto it2 = rank_to_host.find(dst);

                        temp_src = route_table[temp_src][dst];
                        count++;
                        assert(count <= 10);
                        max_for_times = max(max_for_times, count);
                    }
                }
            }
            cout << "max forward times: " << max_for_times + 1 << endl;
        }

        static inline gpu_size_type get_start_idx_per_node(const gpu_size_type &gpu_idx) {
            return gpu_idx - gpu_idx % 4;
        }

    private:
        vector<vector<gpu_size_type>> physical_group;
        vector<vector<gpu_size_type>> route_table;
    };


    void GenerateRouteByPhysicalTopology::get_init_physical_group() {
        for (auto &kv: host_to_rank) {
            cout << kv.first << " * " << kv.second << endl;
        }

        auto dim_sq = static_cast<gpu_size_type>(sqrt(pro_size));    //这个维度要是4的倍数

        while (dim_sq % 4 != 0) { dim_sq++; }


        size_t route_table_len{};
        for (auto &um: first_level_node) {
            for (auto &kv: um.second) {
                std::sort(kv.second.begin(), kv.second.end());    //相同节点得编号排列在一起，这个后续可以优化
                route_table_len += (kv.second.size() / dim_sq + 1);
            }
        }


        physical_group.resize(route_table_len);

        for (auto &um: first_level_node) {
            std::cout << um.first << " ";

            size_t count = 0;
            for (auto &kv: um.second) {

                gpu_size_type idx = 0;
                while (idx < kv.second.size()) {
                    auto num_count = host_to_rank.count(kv.second[idx]);
                    auto it = host_to_rank.find(kv.second[idx]);

                    for (auto num = 0; num < num_count; ++num, ++idx, ++it) {
                        physical_group[count + idx / dim_sq].push_back(it->second);
                    }
                }
                count = (1 + kv.second.size() / dim_sq);
            }
        }
        for (auto &e: physical_group) {
            for (auto e2: e) {
                cout << e2 << " ";
            }
            cout << endl;
        }
    }

    auto GenerateRouteByPhysicalTopology::find_idx(const unsigned int &send_idx) {
        for (int idx = 0; idx < physical_group.size(); ++idx) {
            auto it = std::find(physical_group[idx].begin(), physical_group[idx].end(), send_idx);
            if (it != physical_group[idx].end()) { return make_pair(idx, it); }

        }
        return make_pair(-1, physical_group[0].end());      //正常不会执行这条
    }


    void GenerateRouteByPhysicalTopology::generate_route_by_physics_topology() {
        gpu_size_type len = pro_size;
        route_table.assign(len, vector<unsigned>(len, 0));
        for (gpu_size_type send_idx = 0; send_idx < len; ++send_idx) {
            auto send_idx_it = find_idx(send_idx);

            for (gpu_size_type recv_idx = 0; recv_idx < len; ++recv_idx) {
                auto recv_idx_it = find_idx(recv_idx);
                if (route_table[send_idx][recv_idx]) { continue; }

                if (send_idx_it.first == recv_idx_it.first) {     //在同一行
                    if (is_in_same_node(send_idx, recv_idx)) {
                        route_table[send_idx][recv_idx] = send_idx;       //直接发送
                    } else {
                        route_table[send_idx][recv_idx] = get_start_idx_per_node(recv_idx);       //通过同一节点内的间接发送
                    }
                } else {   //不在同一行,随机挑选一个发送,本行的所有节点都会通过这个随机节点发送
                    auto forward_idx = physical_group[recv_idx_it.first][rand() %
                                                                         (physical_group[recv_idx_it.first].size())];
                    for (auto &idx: physical_group[recv_idx_it.first]) {
                        route_table[send_idx][idx] = forward_idx;
                    }
                }
            }

//            for (gpu_size_type recv_idx = 0; recv_idx < len; ++recv_idx) {
//                auto recv_idx_it = find_idx(recv_idx);
//                if (send_idx_it.first == recv_idx_it.first) {     //在同一行
//                    if (is_in_same_node(send_idx, recv_idx)) {
//                        route_table[send_idx][recv_idx] = send_idx;       //直接发送
//                    } else {
//                        route_table[send_idx][recv_idx] = get_start_idx_per_node(recv_idx);       //通过同一节点内的间接发送
//                    }
//                } else {     //不在同一行,随机挑选一个发送
//                    auto idx = physical_group[recv_idx_it.first][rand() % (physical_group[recv_idx_it.first].size())];
//
//                    route_table[send_idx][recv_idx] = get_start_idx_per_node(idx);
//                }
//            }
        }

        cout << "-------" << len << endl;
        for (gpu_size_type i = 0; i < len; ++i) {
            for (gpu_size_type j = 0; j < len; ++j) {
                cout << route_table[i][j] << " ";
            }
            cout << endl;
        }
        cout << "end!!" << endl;
    }

    GenerateRouteByPhysicalTopology::GenerateRouteByPhysicalTopology(int argc, char **argv) : MpiNodeInfo(argc, argv) {}


}