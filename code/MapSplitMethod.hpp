#pragma once

#include <vector>
#include <unordered_map>
#include "LoadData.hpp"


namespace dtb {
    class MapSplitMethod {
    private:
        unsigned step = 5;     //迭代的步数

        std::vector<gpu_size_type> changed_gpu_idx{};   //本次迭代更改的gpu
        std::vector<std::unordered_map<gpu_size_type, double> > map_table_before_change{};
        std::shared_ptr<LoadData> load_data_instance = LoadData::getLoadDataInstance();
    public:
        template<size_t size>
        void split_pop_by_swap_max_min_pop(const std::array<traffic_size_type, size> &traffic_table);

        void print_split_result();

        const std::vector<gpu_size_type> &getChangedGpuIdx() const;

        std::vector<std::unordered_map<gpu_size_type, double>> &getMapTableBeforeChange();

        static std::shared_ptr<MapSplitMethod> getInstance();

        MapSplitMethod(const MapSplitMethod &) = delete;

        MapSplitMethod &operator=(const MapSplitMethod &) = delete;

    private:

        static std::shared_ptr<MapSplitMethod> instance_ptr;     //单例模式中唯一的实例

        MapSplitMethod() = default;

    };

    std::shared_ptr<MapSplitMethod> MapSplitMethod::instance_ptr = nullptr;

    std::shared_ptr<MapSplitMethod> MapSplitMethod::getInstance() {
        if (!instance_ptr) {
            instance_ptr = std::shared_ptr<MapSplitMethod>(new MapSplitMethod());
        }
        return instance_ptr;
    }


    void MapSplitMethod::print_split_result() {
        std::cout << "The gpus changed in this iteration are: ";
        std::for_each(changed_gpu_idx.begin(), changed_gpu_idx.end(), [](auto i) { std::cout << i << " "; });
        std::cout << std::endl;
        for (unsigned int &it: changed_gpu_idx) {
            std::cout << "gpu index: " << it << std::endl;
            std::cout << "before change: " << std::endl;
            for (auto &pop_pair: map_table_before_change[it]) {
                std::cout << pop_pair.first << " : " << pop_pair.second << " ,";
            }
            std::cout << std::endl;
            std::cout << "after change: " << std::endl;
            for (auto &pop_pair: load_data_instance->getMapTable()[it]) {
                std::cout << pop_pair.first << " : " << pop_pair.second << " ,";
            }
            std::cout << std::endl;
        }
    }

    const std::vector<gpu_size_type> &MapSplitMethod::getChangedGpuIdx() const {
        return changed_gpu_idx;
    }

    std::vector<std::unordered_map<gpu_size_type, double>> &MapSplitMethod::getMapTableBeforeChange() {
        return map_table_before_change;
    }


    template<size_t size>
    void
    MapSplitMethod::split_pop_by_swap_max_min_pop(
            const std::array<traffic_size_type, size> &traffic_table) {   //这个函数需要更改
        changed_gpu_idx.clear();
        std::copy(load_data_instance->getMapTable().begin(), load_data_instance->getMapTable().end(),
                  std::back_inserter(map_table_before_change));
        auto sort_indices_ptr = argsort(traffic_table);
        unsigned iter_indices = 0, iter_count = 0;
        while (iter_count < step) {
            auto max_traffic_gpu_idx = (*sort_indices_ptr)[GPU_NUM - iter_indices - 1];  //从最大开始向后查找
            auto min_traffic_gpu_idx = (*sort_indices_ptr)[iter_indices];
            iter_indices++;

            std::vector<std::pair<decltype(load_data_instance->getMapTable()[max_traffic_gpu_idx].begin()), double>> pop_size;
            bool flag = false;
            for (auto it = load_data_instance->getMapTable()[max_traffic_gpu_idx].begin();
                 it != load_data_instance->getMapTable()[max_traffic_gpu_idx].end(); it++) {
                if (it->second == 1) {
                    pop_size.push_back(std::make_pair(it, load_data_instance->getSizeTable()[it->first]));
                    flag = true;
                }
            }
            if (pop_size.empty()) {
                printf("max traffic gpu index : %d ,but this gpu cannot be split\n", max_traffic_gpu_idx);
                continue;
            }
            std::sort(pop_size.begin(), pop_size.end(), [](auto &a, auto &b) {
                return a.second > b.second;
            });

            iter_count += 1;


//            auto max_pop_iter = std::max_element(pop_size.begin(), pop_size.end());
            if (flag) {

//                auto max_pop_idx = std::find_if(load_data_instance->getMapTable()[max_traffic_gpu_idx].begin(),
//                                                load_data_instance->getMapTable()[max_traffic_gpu_idx].end(),
//                                                [pop_size = *max_pop_iter](std::pair<int, double> pop) {
//                                                    return pop.first == 1 && pop.second == pop_size;
//                                                });
//                if (max_pop_idx != load_data_instance->getMapTable()[max_traffic_gpu_idx].end()) {
//
//                }

//                    load_data_instance->getMapTable().erase(max_pop_idx);
                changed_gpu_idx.push_back(max_traffic_gpu_idx);
                auto start_gpu_idx = 4 * (min_traffic_gpu_idx / 4);
                auto end_gpu_idx = start_gpu_idx + 4;

                for (auto i = start_gpu_idx; i != end_gpu_idx; ++i) {
                    changed_gpu_idx.push_back(i);
//                    std::cout << typeid(*max_pop_idx).name() << std::endl;
                    load_data_instance->getMapTable()[i][(*(pop_size[0].first)).first] = 0.25;
                }
                load_data_instance->getMapTable()[max_traffic_gpu_idx].erase(pop_size[0].first);
            }
        }
    }
}

















