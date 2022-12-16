#pragma once

#include <vector>
#include <unordered_map>
#include "LoadData.hpp"


namespace dtb {
    class mapSplitMethod {
    private:
        int step = 5;     //迭代的步数
        std::vector<unsigned> changed_gpu_idx;   //本次迭代更改的gpu
        std::vector<std::unordered_map<int, double> > map_table_before_change;


        template<size_t size>
        void split_pop_by_swap_max_min_pop(std::array<unsigned long, size> const &traffic_table);

    };

    template<size_t size>
    void mapSplitMethod::split_pop_by_swap_max_min_pop(const std::array<unsigned long, size> &traffic_table) {
        auto load_data_instance = LoadData::getLoadDataInstance();
        std::copy(load_data_instance->getMapTable().begin(), load_data_instance->getMapTable().end(),
                  std::back_inserter(map_table_before_change));
        auto &sort_indices = argsort(traffic_table);
        unsigned iter_indices = 0, iter_count = 0;
        while (iter_count < step) {
            auto max_traffic_gpu_idx = sort_indices[GPU_NUM - iter_indices - 1];  //从最大开始向后查找
            auto min_traffic_gpu_idx = sort_indices[iter_indices];
            iter_indices++;
            std::vector<double> pop_size;
            bool flag = false;
            for (auto it = load_data_instance->getMapTable()[max_traffic_gpu_idx].begin();
                 it != load_data_instance->getMapTable()[max_traffic_gpu_idx].end(); it++) {
                if (it->second == 1) {
                    pop_size.push_back(load_data_instance->getSizeTable()[it->first]);
                    flag = true;
                }
            }
            if (pop_size.empty()) {
                printf("max traffic gpu index : %d ,this gpu cannot be split\n", max_traffic_gpu_idx);
                continue;
            }
            iter_count += 1;
            auto max_pop_iter = std::max_element(pop_size.begin(), pop_size.end());
            if (flag == 1) {

                auto max_pop_idx = std::find_if(load_data_instance->getMapTable()[max_traffic_gpu_idx],
                                                load_data_instance->getMapTable()[max_traffic_gpu_idx],
                                                [pop_size = *max_pop_iter](std::pair<int, double> pop) {
                                                    return pop.first == 1 && pop.second == pop_size;
                                                });

                if (max_pop_idx != load_data_instance->getMapTable()[max_traffic_gpu_idx].end()) {
                    load_data_instance->getMapTable().erase(max_pop_idx);
                    changed_gpu_idx.push_back(max_traffic_gpu_idx);
                    auto start_gpu_idx = 4 * (min_traffic_gpu_idx / 4);
                    auto end_gpu_idx = start_gpu_idx + 4;
                    for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                        changed_gpu_idx.push_back(i);
                        load_data_instance->getMapTable()[i].insert_or_assign(max_pop_idx, 0.25);
                    }
                }
            }
        }
    }
}

















