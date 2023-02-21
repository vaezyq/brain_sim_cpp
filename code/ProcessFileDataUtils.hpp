#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include "BaseInfo.hpp"

namespace dtb {

    template<typename TableData>
    void save_data_to_file_test(std::string &file_path, const TableData &table_data) {
        std::ofstream FILE(file_path, std::ios::out | std::ofstream::binary);

        // 存储vector的大小
        gpu_size_type s1 = table_data.size();
        FILE.write(reinterpret_cast<const char *>(&s1), sizeof(s1));

        // 依次存储table的每一行
        for (auto &v: table_data) {
            // 存储本行的大小
            gpu_size_type size = v.size();
            FILE.write(reinterpret_cast<const char *>(&size), sizeof(size));

            // 存储每行的数据
            FILE.write(reinterpret_cast<const char *>(&v[0]), v.size() * sizeof(gpu_size_type));
        }
        FILE.close();
    }

    template<typename TableData>
    void load_data_from_file_test(std::string &file_path, TableData &table_data) {

        std::ifstream FILE(file_path, std::ios::in | std::ifstream::binary);
        table_data.clear();

        gpu_size_type size = 0;
        //加载table的大小
        FILE.read(reinterpret_cast<char *>(&size), sizeof(size));
        table_data.resize(size);
        for (auto &v: table_data) {
            // 加载vector的大小
            gpu_size_type size2 = 0;
            FILE.read(reinterpret_cast<char *>(&size2), sizeof(size2));
            v.resize(size2);

            // 加载本行的数据
            FILE.read(reinterpret_cast< char *>(&v[0]), v.size() * sizeof(gpu_size_type));
        }
        FILE.close();
    }

    template<typename RouteTable>
    void load_txt_route_file(std::string &route_table_file_path, RouteTable &default_route_table) {
        try {
            std::ifstream route_table_file;
            std::string line;
            route_table_file.open(route_table_file_path);
            route_table_file.exceptions(std::ifstream::badbit);
            gpu_size_type idx = 0;
            while (getline(route_table_file, line)) {  //每一行都是空格隔开的数字(含有GPU_NUM行)
                /*
                 * todo: 这里只能读取默认路由表，因为每一行必须含有GPU_NUM个数字
                 * 因此这里路由表名采用的是default_route_table,后续若出现不规整路由表则可以定义route_table
                 */
                std::string item;
                std::stringstream text_stream(line);
                gpu_size_type i = 0;
                while (getline(text_stream, item, ' ')) {
                    default_route_table[idx][i++] = stoi(item);     //这里应该是转成unsigned,但是没有这个函数，转成int在这里也不会出错
                }
                idx++;
            }
            route_table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load route table file " << route_table_file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    template<typename RouteTable>
    void save_txt_route_file(std::string &route_table_file_path, RouteTable &default_route_table_ptr,
                             const std::vector<gpu_size_type> &dim) {


        try {
            std::ofstream outFile;
            //打开文件
            outFile.open(route_table_file_path, std::ios::out);
            outFile.exceptions(std::ofstream::badbit);
            if (!outFile.is_open()) {
                std::cout << "open error!" << std::endl;
                return;
            }

            int N = std::accumulate(dim.begin(), dim.end(), 1, [](int a, int b) { return a * b; });   //节点数目为累乘结果
            for (gpu_size_type i = 0; i < N; ++i) {
                for (gpu_size_type j = 0; j < N; ++j) {
                    outFile << (*default_route_table_ptr)[i][j] << " ";
                }
                outFile << std::endl;
            }
            //关闭文件
            outFile.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load route table file " << route_table_file_path << " failed," << e.what() << std::endl;
            exit(1);
        }

    }

}

