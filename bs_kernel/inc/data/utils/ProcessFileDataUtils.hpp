/*******************************************************************************
 * @file
 * @brief  读取所有表数据的工具函数，支持读取和写入二进制与txt文本格式的数据
 * @details
 * @author
 * @date       2023-02-23
 * @version    V2.0
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

namespace dtb {


    /*!
     * 将数据写入到二进制文件中(本函数只适用于二维数组的写入,目前用于route_table的写入,一维需要新增函数)
     * @tparam TableData  二维表类型
     * @param file_path   写入目标文件路径
     * @param table_data  表数据
     */
    template<typename TableData>
    void save_two_dim_data_to_binary_file(const std::string &file_path, const TableData &table_data);


    /*!
     * 将数据从二进制文件按中读取,(本函数只适用于二维数组的写入,同时每行数组的大小必须固定，若不固定则需要使用变长的函数)
     * @tparam TableData 二维表类型
     * @param file_path 读取目标文件路径
     * @param table_data 表数据
     */
    template<typename TableData>
    void load_two_dim_data_from_binary_file(const std::string &file_path, TableData &table_data);


    /*!
     * 将数据从二进制文件按中读取,(本函数只适用于二维数组的写入,同时每行数组的大小必须固定，若不固定则需要使用变长的函数)
     * @tparam TableData  二维表类型
     * @param file_path   读取目标文件路径
     * @param table_data  表数据
     */
    template<typename TableData>
    void load_two_dim_lengthening_data_from_binary_file(const std::string &file_path, TableData &table_data);


    /*!
     * 将数据从二进制文件中读取,本函数只适用于一维数组的写入
     * @tparam TableData  一维表类型
     * @param file_path  读取目标文件路径
     * @param table_data 表数据
     */
    template<typename TableData>
    void load_one_dim_data_from_binary_file(const std::string &file_path, TableData &table_data);


    /*!
     * 将数据写入到二进制文件,本函数只适用于一维数组的写入
     * @tparam TableData 一维表类型
     * @param file_path  读取目标文件路径
     * @param table_data 表数据
     */
    template<typename TableData>
    void save_one_dim_data_to_binary_file(const std::string &file_path, const TableData &table_data);

    /*!
    * 将txt格式的路由表文件读取到二维数组中(txt格式为平时测试易于显示，但是读取较慢，建议用二进制的文件),目前仅限于route，因为里面使用了stoi函数
    * @tparam RouteTable  二维路由表类型
    * @param route_table_file_path 读取目标文件路径
    * @param default_route_table 路由表数据
    */
    template<typename RouteTable>
    void
    load_tow_dim_route_data_from_txt_file(const std::string &route_table_file_path, RouteTable &default_route_table);


    /*!
    * 将txt格式的路由表文件写入到二维数组中(txt格式为平时测试易于显示，但是读取较慢，建议用二进制的文件)
    * @tparam RouteTable 二维路由表类型
    * @param route_table_file_path 写入目标文件路径
    * @param default_route_table_ptr 路由表数据
    * @param dim 路由表的每层的维度
    */
    template<typename RouteTable>
    void
    save_two_dim_route_data_to_txt_file(const std::string &route_table_file_path,
                                        const RouteTable &default_route_table_ptr,
                                        const std::vector<gpu_size_type> &dim);


    /*!
     * 将一行一个元素的txt读取到一维容器中
     * @tparam TableData 表类型
     * @param file_path 目标文件路径
     * @param table_data 表数据
     */
    template<typename TableData>
    void load_one_dim_data_from_txt_file(const std::string &file_path, TableData &table_data);


    /*!
     *
     * @tparam TableData
     * @param file_path
     * @param table_data
     */
    template<typename TableData>
    void save_one_dim_data_to_txt_file(const std::string &file_path, const TableData &table_data);


    /*!
     *
     * @tparam T
     * @param data
     * @param file_path
     */
    template<typename T>
    void save_map_data_to_txt_file(std::string const &file_path, const T &data);


    template<typename TableData>
    void save_two_dim_data_to_binary_file(const std::string &file_path, const TableData &table_data) {
        try {
            std::ofstream table_file(file_path, std::ios::out | std::ofstream::binary);
            table_file.exceptions(std::ifstream::badbit);
            // 存储vector的大小
            gpu_size_type s1 = table_data.size();
            table_file.write(reinterpret_cast<const char *>(&s1), sizeof(s1));

            // 依次存储table的每一行
            for (auto &v: table_data) {
                // 存储本行的大小
                gpu_size_type size = v.size();
                table_file.write(reinterpret_cast<const char *>(&size), sizeof(size));
                // 存储每行的数据
                table_file.write(reinterpret_cast<const char *>(&v[0]), v.size() * sizeof(v[0]));
            }
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }

    template<typename TableData>
    void load_two_dim_data_from_binary_file(const std::string &file_path, TableData &table_data) {
        try {
            std::ifstream table_file(file_path, std::ios::in | std::ifstream::binary);
            table_file.exceptions(std::ifstream::badbit);

            gpu_size_type size = 0;
            //加载table的大小
            table_file.read(reinterpret_cast<char *>(&size), sizeof(size));

            for (auto &v: table_data) {
                // 加载本行的数据
                gpu_size_type size2 = 0;
                table_file.read(reinterpret_cast<char *>(&size2), sizeof(size2));
                table_file.read(reinterpret_cast< char *>(&v[0]), v.size() * sizeof(v[0]));
            }
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    template<typename TableData>
    void load_two_dim_lengthening_data_from_binary_file(const std::string &file_path, TableData &table_data) {
        try {
            std::ifstream table_file(file_path, std::ios::in | std::ifstream::binary);
            table_file.exceptions(std::ifstream::badbit);
            table_data.clear();

            gpu_size_type size = 0;
            //加载table的大小
            table_file.read(reinterpret_cast<char *>(&size), sizeof(size));
            table_data.resize(size);
            for (auto &v: table_data) {
                // 加载vector的大小
                gpu_size_type size2 = 0;
                table_file.read(reinterpret_cast<char *>(&size2), sizeof(size2));
                v.resize(size2);

                // 加载本行的数据
                table_file.read(reinterpret_cast< char *>(&v[0]), v.size() * sizeof(v[0]));
            }
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    template<typename TableData>
    void load_one_dim_data_from_binary_file(const std::string &file_path, TableData &table_data) {
        try {
            std::ifstream table_file(file_path, std::ios::in | std::ifstream::binary);
            table_file.exceptions(std::ifstream::badbit);
            table_file.read(reinterpret_cast< char *>(&table_data[0]), table_data.size() * sizeof(table_data[0]));
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    template<typename TableData>
    void save_one_dim_data_to_binary_file(const std::string &file_path, const TableData &table_data) {
        try {
            std::ofstream table_file(file_path, std::ios::out | std::ofstream::binary);
            table_file.exceptions(std::ifstream::badbit);
            table_file.write(reinterpret_cast<const char *>(&table_data[0]), table_data.size() * sizeof(table_data[0]));
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    template<typename RouteTable>
    void
    load_tow_dim_route_data_from_txt_file(const std::string &route_table_file_path, RouteTable &default_route_table) {
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
    void
    save_two_dim_route_data_to_txt_file(const std::string &route_table_file_path, const RouteTable &default_route_table,
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
                    outFile << (default_route_table)[i][j] << " ";
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


    template<typename TableData>
    void load_one_dim_data_from_txt_file(const std::string &file_path, TableData &table_data) {
        try {
            std::ifstream file_data;
            std::string line;
            file_data.open(file_path);        //将文件读取到流中
            file_data.exceptions(std::ifstream::badbit);

            std::generate(table_data.begin(), table_data.end(), [&file_data, &line]()mutable {
                getline(file_data, line);                                          //每一行数据只包含一个double类型，将其读取后用过strtod转换
                return strtod(line.substr(0, line.size()).c_str(), nullptr);
            });
//            int idx = 0;
//            while (getline(file_data, line)) {      //本文件每一行都会包含一个double对象
//                data[idx++] =
//            }
        } catch (std::ios_base::failure &e) {
            std::cout << "Read file failed," << e.what() << std::endl;
//            std::terminate();
            exit(1);
        }
    }


    template<typename TableData>
    void save_one_dim_data_to_txt_file(const std::string &file_path, const TableData &table_data) {
        try {
            std::ofstream file_save_data(file_path);
            int idx = 0;
            for (auto const &x: table_data) {
                file_save_data << x << "\n";
                idx++;
            }
            file_save_data.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "write result to file failed," << e.what() << std::endl;
            std::terminate();
        }
    }


    template<typename T>
    void save_map_data_to_txt_file(const std::string &file_path, const T &data) {
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

    void save_conn_data_to_binary_file(const std::string &file_path,
                                       const std::unordered_map<double, double> &conn_dict_table) {
        try {
            std::ofstream table_file(file_path, std::ios::out | std::ofstream::binary);
            table_file.exceptions(std::ifstream::badbit);
            std::vector<double> key(conn_dict_table.size(), 0);
            std::vector<double> value(conn_dict_table.size(), 0);

            gpu_size_type s = conn_dict_table.size();  //存储大小
            table_file.write(reinterpret_cast<const char *>(&s), sizeof(s));

            transform(conn_dict_table.begin(), conn_dict_table.end(), key.begin(), [](auto &p) {
                return p.first;
            });

            transform(conn_dict_table.begin(), conn_dict_table.end(), value.begin(), [](auto &p) {
                return p.second;
            });

            table_file.write(reinterpret_cast<const char *>(&key[0]), key.size() * sizeof(key[0]));
            table_file.write(reinterpret_cast<const char *>(&value[0]), key.size() * sizeof(value[0]));
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }

    void load_conn_data_from_binary_file(const std::string &file_path,
                                         std::unordered_map<double, double> &conn_dict_table) {
        try {
            std::ifstream table_file(file_path, std::ios::in | std::ifstream::binary);
            table_file.exceptions(std::ifstream::badbit);

            gpu_size_type size = 0;

            //加载table的大小
            table_file.read(reinterpret_cast<char *>(&size), sizeof(size));

            std::cout << size << std::endl;
            std::vector<double> key(size, 0);
            std::vector<double> value(size, 0);

            table_file.read(reinterpret_cast< char *>(&key[0]), size * sizeof(double));

            table_file.read(reinterpret_cast< char *>(&value[0]), size * sizeof(double));

            for (auto iter = key.begin(), iter2 = value.begin(); iter != key.end();
                 advance(iter, 1), advance(iter2, 1)) {
                conn_dict_table.emplace(*iter, *iter2);
            }
            table_file.close();

        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    //std::vector<std::unordered_map<gpu_size_type, double> > map_table;
    void save_map_data_to_binary_file(const std::string &file_path,
                                      const std::vector<std::unordered_map<gpu_size_type, double> > &map_table) {
        try {
            std::ofstream table_file(file_path, std::ios::out | std::ofstream::binary);
            table_file.exceptions(std::ifstream::badbit);

            for (auto it = map_table.begin(); it != map_table.end(); advance(it, 1)) {
                gpu_size_type s = it->size();  //存储大小
                table_file.write(reinterpret_cast<const char *>(&s), sizeof(s));

                std::vector<gpu_size_type> key(s, 0);
                std::vector<double> value(s, 0);
                transform(it->begin(), it->end(), key.begin(), [](auto &p) {
                    return p.first;
                });
                transform(it->begin(), it->end(), value.begin(), [](auto &p) {
                    return p.second;
                });

                table_file.write(reinterpret_cast<const char *>(&key[0]), key.size() * sizeof(key[0]));
                table_file.write(reinterpret_cast<const char *>(&value[0]), key.size() * sizeof(value[0]));

            }

            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    void load_map_data_from_binary_file(const std::string &file_path,
                                        std::vector<std::unordered_map<gpu_size_type, double> > &map_table) {
        try {
            std::ifstream table_file(file_path, std::ios::in | std::ifstream::binary);
            table_file.exceptions(std::ifstream::badbit);


            map_table.resize(GPU_NUM);
            gpu_size_type size = 0;
            for (auto it = map_table.begin(); it != map_table.end(); advance(it, 1)) {
                //加载table的大小
                table_file.read(reinterpret_cast<char *>(&size), sizeof(size));

                std::vector<gpu_size_type> key(size, 0);
                std::vector<double> value(size, 0);

                table_file.read(reinterpret_cast< char *>(&key[0]), size * sizeof(gpu_size_type));
                table_file.read(reinterpret_cast< char *>(&value[0]), size * sizeof(double));
                auto iter = key.begin();
                auto iter2 = value.begin();
                for (; iter != key.end(); advance(iter, 1), advance(iter2, 1)) {
                    it->emplace(*iter, *iter2);
                }
            }
            table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load table file " << file_path << " failed," << e.what() << std::endl;
            exit(1);
        }
    }


    template<typename T>
    void load_one_dim_traffic_two_dim_result(const std::string &file_path, T &one_dim_traffic) {
        std::ifstream traffic_table_file;
        std::string line;

        traffic_table_file.open(file_path);
        if (!traffic_table_file.is_open()) {
            std::cout << "traffic table file open failed" << std::endl;
            return;
        }
        gpu_size_type idx = 0;
        while (getline(traffic_table_file, line)) {  //每一行都是空格隔开的数字(含有GPU_NUM行)
            std::string item;
            std::stringstream text_stream(line);
            gpu_size_type i = 0;
            while (std::getline(text_stream, item, ' ')) {
                one_dim_traffic[idx][i++] = stod(item);
            }
            idx++;
        }
        traffic_table_file.close();
    }


}

