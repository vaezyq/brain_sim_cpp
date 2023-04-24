/*******************************************************************************
 * @file
 * @brief  提供剩余杂乱工具函数
 * @details
 * @author
 * @date       2022-12-05
 * @version    V1.0
 *******************************************************************************/

#pragma once


#include <iostream>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <memory>
#include <vector>
#include <random>
#include <ctime>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <array>
#include <fstream>
#include <sstream>
#include "data/BaseInfo.hpp"


using namespace std::literals;
namespace dtb {


    /*!
    * 判断两个卡是否在同一节点内
    * @param gpu_a  卡a编号
    * @param gpu_b  卡b编号
    * @return 在一个卡内则返回true,否则返回false
    */
    inline bool is_in_same_node(const gpu_size_type &gpu_a, const gpu_size_type &gpu_b) {
        if (gpu_a / 4 == gpu_b / 4) {
            return true;
        }
        return false;
    }

    /*!
    * 根据采样范围和采样次数得到采样结果(可以并行优化)
    * @param sample_range 采样范围
    * @param sample_times 采样次数
    * @return 采样数组唯一化后的大小
    */
    //17w*10 170w
    //todo 并行或采用cuda优化这个函数
    traffic_size_type sample(const traffic_size_type &sample_range, const traffic_size_type &sample_times);



    /*!
     *
     * @tparam T
     * @param traffic_output_iter_map
     * @param traffic_input_iter_map
     * @param traffic_file_path
     * @param dimension
     */
    template<typename T>
    void
    load_traffic_for_gen_map(T &traffic_output_iter_map, T &traffic_input_iter_map,
                             const std::string &traffic_file_path, unsigned dimension);


    /*!
     * c++实现的argsort,得到所有排序后序列的索引
     * @tparam T 元素类型
     * @param array
     * @return
     */
    template<typename T, size_t size>
    std::unique_ptr<std::array<T, size>> argsort(const std::array<T, size> &traffic_table);

    template<typename T>
    void
    write_map_data_to_file(const T &data, std::string const &file_path);




    // (0,50000)  200000   5w

    traffic_size_type sample(const traffic_size_type &sample_range, const traffic_size_type &sample_times) {
        srand(time(nullptr));
        std::unordered_set<unsigned long> random_sample;
        for (unsigned i = 0; i < sample_times; ++i) {
//        std::uniform_int_distribution<int> dist(0, sample_range);
            auto sample_range_int = static_cast<long long >(sample_range);
            if (auto result = rand() % sample_range_int;!random_sample.count(result)) {
                random_sample.insert(result);
            }
        }
        return random_sample.size();
    }




    template<size_t size>
    void print_max_min_aver_traffic_info(const std::array<traffic_size_type, size> &traffic_table) {

        auto max_iter = std::max_element(traffic_table.cbegin(), traffic_table.cend());
        auto min_iter = std::min_element(traffic_table.cbegin(), traffic_table.cend());
        std::cout << "max traffic: " << *max_iter << " ,max traffic gpu idx: "
                  << std::distance(traffic_table.begin(), max_iter) << std::endl;

        std::cout << "min traffic: " << *min_iter << " ,min traffic gpu idx: "
                  << std::distance(traffic_table.begin(), min_iter) << std::endl;

        //这里要使用long long类型，否则相加会出现溢出
        auto average = std::accumulate(traffic_table.begin(), traffic_table.end(), 0ULL) / size;
        std::cout << "average traffic: " << average << std::endl;
    }



    template<typename T, size_t size>
    std::unique_ptr<std::array<T, size>> argsort(const std::array<T, size> &traffic_table) {
        auto res_ptr = std::make_unique<std::array<T, size>>(std::array<T, size>());
        std::iota(res_ptr->begin(), res_ptr->end(), 0);
        std::sort(res_ptr->begin(), res_ptr->end(), [&traffic_table](int left, int right) {
            return traffic_table[left] < traffic_table[right];
        });
        return res_ptr;
    }

}

