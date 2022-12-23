//
// Created by 王明龙 on 2022/12/5.
//
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
#include "BaseInfo.hpp"

namespace dtb {

    using gpu_size_type = unsigned;  //所有gpu卡编号采用unsigned,目前卡编号远小于这个数值

    using pop_size_type = unsigned;  //所有pop编号采用unsigned,目前体素大概十万,64位系统远远满足

    using neuron_size_type = double;   //神经元数目目前为亿级别，最大为860亿，可采用double表示

    using traffic_size_type = double;     //目前的流量统计，使用unsigned一定会溢出，也可以使用unsigned long long,这里直接使用double


    /*!
    * 传入一个vector,以及文件路径，将文件内数据读入vector中
    * @tparam T  vector需要存储的元素类型
    * @param data 读取后写入的目标容器
    * @param file_path 读取文件路径
    * @param len 读取长度
     */
    template<typename T, size_t N>
    void load_vector_data(std::array<T, N> &data, const std::string &file_path, int len);


    /*!
     *
     * @tparam T vector的数据类型，此类型需要重载了输出运算符<<
     * @param data vector
     * @param file_path 存入的目标文件
     */
    template<typename T>
    void write_vector_data_file(const T &data, std::string const &file_path, unsigned col_len);

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
     * 根据输入的流量表打印这个流量数据的最大值、最小值、平均值
     * @tparam size 流量表的大小
     * @param traffic_table 流量表
     */
    template<size_t size>
    void print_max_min_aver_traffic_info(const std::array<traffic_size_type, size> &traffic_table);


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
                             const std::string &traffic_file_path, const unsigned dimension);


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


    template<typename T, size_t N>
    void load_vector_data(std::array<T, N> &data, const std::string &file_path, const int len) {
        try {
            std::ifstream file_data;
            std::string line;
            file_data.open(file_path);
            file_data.exceptions(std::ifstream::badbit);
            int idx = 0;
            while (getline(file_data, line)) {
                data[idx++] = strtod(line.substr(0, line.size()).c_str(), nullptr);
            }
        } catch (std::ios_base::failure &e) {
            std::cout << "write result to file failed," << e.what() << std::endl;
            std::terminate();
        }
    }


    template<typename T>
    void write_vector_data_file(const T &data, std::string const &file_path, unsigned col_len) {
        try {
            std::ofstream file_save_data(file_path);
            int idx = 0;
            for (auto const &x: data) {
                file_save_data << x << " ";
                idx++;
                if (idx % col_len == 0) {
                    file_save_data << '\n';
                }
            }
            file_save_data.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "write result to file failed," << e.what() << std::endl;
            std::terminate();
        }
    }

    traffic_size_type sample(const traffic_size_type &sample_range, const traffic_size_type &sample_times) {
        srand(time(nullptr));
        std::unordered_set<unsigned long> random_sample;
        for (unsigned int i = 0; i < sample_times; ++i) {
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

    template<typename T>
    void
    load_traffic_for_gen_map(T &traffic_output_iter_map, T &traffic_input_iter_map,
                             const std::string &traffic_file_path, const unsigned dimension) {
        try {
            std::ifstream traffic_table_file;
            std::string line;
            traffic_table_file.open(traffic_file_path);

            traffic_table_file.exceptions(std::ifstream::badbit);
            int idx = 0;
            while (getline(traffic_table_file, line)) {
                std::string item;
                std::stringstream text_stream(line);
                int i = 0;
                while (std::getline(text_stream, item, ' ')) {
//                    std::cout << item << std::endl;
                    if (i++ < dimension) {
//                        std::cout << std::stoul(item) << std::endl;
                        traffic_output_iter_map[idx] += stod(item);
                    } else {
                        traffic_input_iter_map[idx] += stod(item);
                    }
                }
                idx++;
            }
            traffic_table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load traffic result file failed," << e.what() << std::endl;
            std::terminate();
        }
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

    template<typename T>
    void
    write_map_data_to_file(const T &data, std::string const &file_path) {
        try {
            std::ofstream file_save_data(file_path);
            for (auto it = data.begin(); it != data.end(); ++it) {
                for (auto const &pop_pair: *it) {
                    file_save_data << pop_pair.first << " " << pop_pair.second << " ";
                }
                file_save_data << "\n";
            }
            file_save_data.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "write result to file failed," << e.what() << std::endl;
            std::terminate();
        }
    }

}

