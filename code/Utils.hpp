//
// Created by 王明龙 on 2022/12/5.
//

#ifndef BRAIN_SIM_UTILS_HPP
#define BRAIN_SIM_UTILS_HPP

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

namespace dtb {
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
    void write_vector_data_file(const T &data, std::string const &file_path);

    /*!
    * 判断两个卡是否在同一节点内
    * @param gpu_a  卡a编号
    * @param gpu_b  卡b编号
    * @return 在一个卡内则返回true,否则返回false
    */
    inline bool is_in_same_node(const unsigned &gpu_a, const unsigned &gpu_b) {
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
    unsigned long sample(unsigned long sample_range, unsigned int sample_times);


    /*!
     * 根据输入的流量表打印这个流量数据的最大值、最小值、平均值
     * @tparam size 流量表的大小
     * @param traffic_table 流量表
     */
    template<size_t size>
    void print_max_min_aver_traffic_info(const std::array<unsigned long, size> &traffic_table);


    /*!
     * 加载流量数据，并计算最大值、最小值、平均值
     * @tparam size  目前计算的流量表大小
     * @param traffic_file_path  流量文件路径
     * @return 返回读取的流量数据
     */
    template<size_t size>
    void
    load_traffic_result(std::array<unsigned long, size> &traffic_table, const std::string &traffic_file_path);


    /*!
     * c++实现的argsort,得到所有排序后序列的索引
     * @tparam T 元素类型
     * @param array
     * @return
     */
    template<typename T, size_t size>
    const std::array<T, size> &argsort(const std::array<T, size> &traffic_table);


    template<typename T, size_t N>
    void load_vector_data(std::array<T, N> &data, const std::string &file_path, const int len) {
        std::ifstream file_data;
        std::string line;
        file_data.open(file_path);
        try {
            int idx = 0;
            while (getline(file_data, line)) {
                data[idx++] = strtod(line.substr(0, line.size()).c_str(), nullptr);
            }
        } catch (std::ios_base::failure &e) {
            std::cout << e.what() << std::endl;
        }
    }


    template<typename T>
    void write_vector_data_file(const T &data, std::string const &file_path) {
        std::ofstream file_save_data(file_path);
        try {
            for (auto const &x: data)
                file_save_data << x << '\n';
            file_save_data.close();
        } catch (std::ios_base::failure &e) {
            std::cout << e.what() << std::endl;
        }
    }

    unsigned long sample(const unsigned long sample_range, const unsigned int sample_times) {
        srand(time(nullptr));
        std::unordered_set<unsigned long> random_sample;
        for (unsigned int i = 0; i < sample_times; ++i) {
//        std::uniform_int_distribution<int> dist(0, sample_range);
            if (auto result = rand() % sample_range;!random_sample.count(result)) {
                random_sample.insert(result);
            }
        }
        return random_sample.size();
    }

    template<size_t size>
    void print_max_min_aver_traffic_info(const std::array<unsigned long, size> &traffic_table) {

        auto max_iter = std::max_element(traffic_table.cbegin(), traffic_table.cend());
        auto min_iter = std::min_element(traffic_table.cbegin(), traffic_table.cend());
        std::cout << "max traffic: " << *max_iter << " ,max traffic gpu idx: "
                  << std::distance(traffic_table.begin(), max_iter) << std::endl;

        std::cout << "min traffic: " << *min_iter << " ,min traffic gpu idx: "
                  << std::distance(traffic_table.begin(), min_iter) << std::endl;

        double average = std::accumulate(traffic_table.begin(), traffic_table.end(), 0) / size;
        std::cout << "average traffic: " << average << std::endl;
    }

    template<size_t size>
    void
    load_traffic_result(std::array<unsigned long, size> &traffic_table, const std::string &traffic_file_path) {
        std::ifstream traffic_table_file;
        std::string line;
        traffic_table_file.open(traffic_file_path);

        if (!traffic_table_file.is_open()) {
            std::cout << "traffic file open failed" << std::endl;
            return;
        }
        int idx = 0;
        while (getline(traffic_table_file, line)) {
            std::stringstream s(line);//将字符串line放入到输入输出流ss中
            s >> traffic_table[idx];
            idx++;
        }
        traffic_table_file.close();
        print_max_min_aver_traffic_info(traffic_table);
    }

    template<typename T, size_t size>
    const std::array<T, size> &argsort(const std::array<T, size> &traffic_table) {

        std::array<T, size> indices;
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&traffic_table](int left, int right) {
            return traffic_table[left] < traffic_table[right];
        });
        return indices;
    }

}


#endif //BRAIN_SIM_UTILS_HPP
