


#pragma once

#include "LoadData.hpp"


namespace dtb {
    /*!
     * 用于串行生成map表
     */
    class GenerateMapUtils {

    public:

        /*!
         * 生成按照顺序排列的map表
         * @return
         */
        static std::shared_ptr<std::vector<std::unordered_map<pop_size_type, double>>> generate_sequential_map();

        /*!
         * 按照max_rate、max_iter_items迭代map表,通过交换较大和较小size*degree，来平衡size*degree
         * 这个函数目前没有写完，因为此函数的做法是通过size*degree，采用贪心的算法迭代线性初始map
         * 此类实现了generate_map_by_force_partition函数，实现了指定参数，例如size*degree的一次分割
         * @param max_rate   平均值和最大值的比例
         * @param max_iter_items   迭代次数
         * @return  返回迭代生成的map表
         */
        std::shared_ptr<std::vector<std::unordered_map<pop_size_type, double>>>
        generate_map_by_balance_size_and_degree(const double &max_rate, const double &max_iter_items);


        /*!
         * 采用暴力算法，根据每个population的size*degree实现一次全部分割
         * @return
         */
        std::shared_ptr<std::array<std::unordered_map<pop_size_type, double>, GPU_NUM> >
        generate_map_by_force_partition();


        /*!
         * 接口返回函数
         * @return 返回每张gpu卡所有population的总和
         */
        [[nodiscard]] const std::array<double, GPU_NUM> &getSizeDegreePreGpu() const;


        /*!
         * 根据指定的map表计算每张卡含有的population的size*degree总和
         * @tparam T 传入的map表类型，因为有时传入的是array，有时是vector，因此这里采用了模板
         * @param map_table
         */
        template<class T>
        void calculate_size_multi_degree(const T &map_table);

    private:

        std::array<double, GPU_NUM> size_degree_pre_gpu{};         //每张卡，所有size*degree的累加和

        /*!
         * generate_map_by_balance_size_and_degree迭代函数 判定迭代终止的辅助函数
         * @param max_rate 判定迭代是否终止的最大值和平均值的比率
         * @param iter_items 迭代步数
         * @return  是否停止迭代
         */
        inline bool iter_stop_flag(const double &max_rate, const double &iter_items);

        /*!
         * 用于生成线性map的辅助函数
         * 例如在2000卡时，结果是{85: 492, 86: 1508}，表示有492张卡有85个population，有1508张卡有86个population
         * @return 得到每张卡需要包含的population个数
         */
        static std::unordered_map<gpu_size_type, pop_size_type> get_pop_nums_per_gpu();

    };

    std::unordered_map<gpu_size_type, pop_size_type> GenerateMapUtils::get_pop_nums_per_gpu() {
        auto pop_nums = POP_NUM / GPU_NUM;
        auto gpu_nums_x2 = POP_NUM - pop_nums * pop_nums * GPU_NUM;
        auto gpu_nums_x = GPU_NUM - gpu_nums_x2;
        std::unordered_map<gpu_size_type, pop_size_type> pops_pre_gpu;
        pops_pre_gpu.insert({pop_nums, gpu_nums_x});
        pops_pre_gpu.insert({pop_nums + 1, gpu_nums_x2});
        return pops_pre_gpu;
    }

    std::shared_ptr<std::vector<std::unordered_map<pop_size_type, double>>>

    GenerateMapUtils::generate_sequential_map() {
        auto pops_pre_gpu = get_pop_nums_per_gpu();
        auto sequential_map = std::shared_ptr<std::vector<std::unordered_map<pop_size_type,
                double >>>();

        pop_size_type pop_idx = 0;
        for (const auto &pop_pair: pops_pre_gpu) {
            for (gpu_size_type i = 0; i < pop_pair.second; ++i) {      //对于每张卡
                std::unordered_map<pop_size_type, double> map_gpu;
                std::generate_n(std::inserter(map_gpu, map_gpu.begin()), pop_pair.first,
                                [&pop_idx]() { return std::make_pair(pop_idx++, 1); });
                sequential_map->emplace_back(std::move(map_gpu));
            }
        }

        auto base_ptr = BaseInfo::getInstance();
        std::string map_file_path =
                base_ptr->map_read_path + "/" + std::to_string(GPU_NUM) + "_" + base_ptr->gene_sequential_map_name +
                ".txt";

        write_map_data_to_file(*sequential_map, map_file_path);
        return sequential_map;
    }


    //todo: 这个函数没有完成，因为最终目的和generate_map_by_force_partition一致
    std::shared_ptr<std::vector<std::unordered_map<pop_size_type, double>>>
    GenerateMapUtils::generate_map_by_balance_size_and_degree(const double &max_rate, const double &max_iter_items) {
        auto sequential_map = generate_sequential_map();
        calculate_size_multi_degree(*sequential_map);
        std::cout << "Begin to generate map: " << std::endl;
        while (iter_stop_flag(max_rate, max_iter_items)) {

        }
        return {};
    }

    template<class T>
    void GenerateMapUtils::calculate_size_multi_degree(const T &map_table) {
        auto load_data_ptr = LoadData::getLoadDataInstance();
        for (gpu_size_type i = 0; i != GPU_NUM; ++i) {
            for (auto &pop_idx_pair: map_table[i]) {
                size_degree_pre_gpu[i] += load_data_ptr->getSizeTable()[pop_idx_pair.first] *
                                          load_data_ptr->getDegreeTable()[pop_idx_pair.first] * pop_idx_pair.second;
            }
        }
    }

    bool GenerateMapUtils::iter_stop_flag(const double &max_rate, const double &iter_items) {
        static unsigned count = 0;
        auto max_ele = *std::max_element(size_degree_pre_gpu.begin(), size_degree_pre_gpu.end());
        auto average = std::accumulate(size_degree_pre_gpu.begin(), size_degree_pre_gpu.end(), 0.0) / GPU_NUM;
        return ((average * max_rate) < max_ele) && (count++ < iter_items);
    }

    std::shared_ptr<std::array<std::unordered_map<pop_size_type, double>, GPU_NUM> >
    GenerateMapUtils::generate_map_by_force_partition() {

//        auto sequential_map = generate_sequential_map();
//        calculate_size_multi_degree(*sequential_map);    //计算得到重平衡的数组
        auto load_data_ptr = LoadData::getLoadDataInstance();
        std::vector<double> size_degree_per_pop(POP_NUM);           //17w个体素的 size*degree的结果

        std::generate(size_degree_per_pop.begin(), size_degree_per_pop.end(), [i = 0, load_data_ptr]()mutable {
            auto res = load_data_ptr->getDegreeTable()[i] * load_data_ptr->getSizeTable()[i];
            i++;
            return res;
        });

        std::fill(size_degree_pre_gpu.begin(), size_degree_pre_gpu.end(), 0);

        auto balance_map = std::array<std::unordered_map<pop_size_type, double>, GPU_NUM>();

        double sum_degree_size = 0.0;
        for (pop_size_type i = 0; i < size_degree_per_pop.size(); ++i) {
            std::cout << i << std::endl;
            double average_before = sum_degree_size / GPU_NUM;
            sum_degree_size += size_degree_per_pop[i];
            auto average = sum_degree_size / GPU_NUM;
            unsigned index = 0;
            for (gpu_size_type idx = 1; idx != GPU_NUM; ++idx) {        //寻找放入后方差最小的gpu
//                std::cout << idx << std::endl;
                if ((pow((size_degree_pre_gpu[idx] + size_degree_per_pop[i] - average), 2) +
                     pow((size_degree_pre_gpu[index] - average_before), 2)) <
                    (pow((size_degree_pre_gpu[index] + size_degree_per_pop[i] - average), 2) +
                     pow((size_degree_pre_gpu[idx] - average_before), 2))) {
                    index = idx;
                }
            }
            std::cout << "selected: " << index << std::endl;
            size_degree_pre_gpu[index] += size_degree_per_pop[i];
            balance_map[index].insert({i, 1});
        }

        auto base_ptr = BaseInfo::getInstance();
        std::string map_file_path =
                base_ptr->map_read_path + "/" + std::to_string(GPU_NUM) + "_" + base_ptr->gene_size_balanced_name +
                ".txt";

        write_map_data_to_file(balance_map, map_file_path);
        return std::make_shared<std::array<std::unordered_map<pop_size_type, double>, GPU_NUM>>(balance_map);
    }

    const std::array<double, GPU_NUM> &GenerateMapUtils::getSizeDegreePreGpu() const {
        return size_degree_pre_gpu;
    }
}









