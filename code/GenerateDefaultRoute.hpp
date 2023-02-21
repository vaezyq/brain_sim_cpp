//
// Created by 王明龙 on 2022/12/5. 生成默认路由表
//
#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <memory>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>
#include "BaseInfo.hpp"
#include "ProcessFileDataUtils.hpp"
#include "AssertUtils.hpp"


namespace dtb {

    class GenerateDefaultRoute {  //生成默认路由表

    public:
        /*!
         * 生成指定维度的路由表的构造函数
         * @param dim 需要生成的维度，生成1800卡2维的，即可以填{45,40}
         */
        explicit GenerateDefaultRoute(const std::vector<gpu_size_type> &dim);

        /*!
         * 生成路由表的主函数
         * @return
         */
        std::unique_ptr<std::vector<std::vector<gpu_size_type >>> generate_specific_default_route();

        /*!
         * 验证生成的路由表的正确性
         */
        static void confirm_route_table(std::vector<std::vector<gpu_size_type >> const &);

        /*!
         *主调用函数，用于生成路由表、验证路由表准确性，并保存路由表到文件
         */
        void generate_confirm_and_save_specific_default_route();


        /*!
         * 主调用函数，用于生成路由表、验证路由表准确性，并保存路由表到txt文件，这个是旧版代码，保存的txt文件易于阅读，但是读写很慢
         */
         [[deprecated("The storage is too slow for txt")]]
        void generate_confirm_and_save_specific_default_route_save_txt();

    private:

        /*!
         * 得到在指定维度dimensions下的每个点的坐标
         * @return 返回一个unique_ptr，其包含在指定维度dim下，N个节点的坐标
         */
        std::unique_ptr<std::vector<std::vector<gpu_size_type >>> get_rank_coordinate();


        /*!
         * get_rank_coordinate的反函数，指定卡坐标和维度计算其编号
         * @param rank_cor 卡坐标
         * @return 卡编号
         */
        unsigned int get_rank_num(std::vector<gpu_size_type> const &rank_cor);

        std::vector<gpu_size_type> dim;   //存储维度

        std::vector<gpu_size_type>::size_type dim_len;   //维度的个数

    };

    GenerateDefaultRoute::GenerateDefaultRoute(const std::vector<gpu_size_type> &dim) : dim(dim) {

        assert_load_data("Dimension error",
                         std::accumulate(dim.begin(), dim.end(), 1, [](int a, int b) { return a * b; }) ==
                         GPU_NUM);     //判断节点数目是否等于模拟的卡数

        dim_len = dim.size();

    }

    std::unique_ptr<std::vector<std::vector<gpu_size_type >>> GenerateDefaultRoute::get_rank_coordinate() {
        auto rank_coordinate_ptr = std::make_unique<std::vector<std::vector<gpu_size_type>>>(
                std::vector<std::vector<gpu_size_type> >(GPU_NUM, std::vector<gpu_size_type>(dim_len, 0))
        );  //每个节点在N维情况下的坐标
        for (gpu_size_type idx = 0; idx != GPU_NUM; ++idx) {
            gpu_size_type num = idx;
            for (int j = static_cast<int>(dim_len) - 1; j != -1; --j) {
                (*rank_coordinate_ptr)[idx][j] = (num % dim[j]);
                num /= dim[j];
            }
        }
        return rank_coordinate_ptr;
    }


    gpu_size_type GenerateDefaultRoute::get_rank_num(const std::vector<gpu_size_type> &rank_cor) {
        int index = static_cast<int>(dim_len - 1);
        unsigned int rank_num = 0, count = 1;
        while (index >= 0) {
            rank_num += (rank_cor[index] * count);
            count *= static_cast<int>(dim[index]);
            index -= 1;
        }
        return rank_num;
    }

    std::unique_ptr<std::vector<std::vector<unsigned >>>
    GenerateDefaultRoute::generate_specific_default_route() {        //为什么这里无法改成array

        auto rank_coordinate_ptr = get_rank_coordinate();    //每个节点在N维情况下的坐标
//        std::vector<std::vector<int> > rank_coordinate(this->N, std::vector<int>(dimensions, 0));  //每个节点在N维情况下的坐标

//        std::array<std::array<unsigned, GPU_NUM>, GPU_NUM> arr{};
//        auto route_table_ptr = std::make_unique<std::array<std::array<unsigned, GPU_NUM>, GPU_NUM>>(
//                arr);   //最终计算得到的转发表
        auto route_table_ptr = std::make_unique<std::vector<std::vector<gpu_size_type>>>(
                std::vector<std::vector<gpu_size_type> >(GPU_NUM,
                                                         std::vector<gpu_size_type>(GPU_NUM, 0)));
        for (gpu_size_type in_rank = 0; in_rank != GPU_NUM; ++in_rank) {
            std::cout << in_rank << std::endl;

            for (gpu_size_type out_rank = 0; out_rank != GPU_NUM; ++out_rank) {
                if (out_rank == in_rank) {
                    (*route_table_ptr)[in_rank][out_rank] = in_rank;
                    continue;
                }
                std::vector<gpu_size_type> in_rank_cor_temp(dim_len, 0);
                std::copy((*rank_coordinate_ptr)[in_rank].begin(), (*rank_coordinate_ptr)[in_rank].end(),
                          in_rank_cor_temp.begin());

                gpu_size_type index = 0;
                while (index < dim_len) {
                    if (in_rank_cor_temp[index] == ((*rank_coordinate_ptr)[out_rank][index])) {
                        index += 1;
                        continue;
                    } else {
                        in_rank_cor_temp[index] = ((*rank_coordinate_ptr)[out_rank][index]);
                        auto rank_num = get_rank_num(in_rank_cor_temp);
                        if (rank_num == out_rank) {
                            (*route_table_ptr)[in_rank][out_rank] = in_rank;
                        } else {
                            (*route_table_ptr)[in_rank][out_rank] = rank_num;
                        }
                        break;
                    }
                }
            }
        }
        return route_table_ptr;
    }

    void GenerateDefaultRoute::confirm_route_table(std::vector<std::vector<gpu_size_type >> const &route_table) {
        std::vector<gpu_size_type> step_length(GPU_NUM * GPU_NUM, 0);

        std::cout << "Begin to confirm route table..." << std::endl;
        for (gpu_size_type src = 0; src < GPU_NUM; ++src) {
            for (gpu_size_type dst = 0; dst < GPU_NUM; ++dst) {
                unsigned temp_src = src;
                while ((route_table)[temp_src][dst] != temp_src) {
                    temp_src = (route_table)[temp_src][dst];
                    step_length[src * GPU_NUM + dst] += 1;
                    assert_load_data("Too many forwards", step_length[src * GPU_NUM + dst] < 10);
                }
            }
            if (src / 1000 == 0) {
                printf("%d / %d \n", src, GPU_NUM);
            }
        }
        auto max_ele = *std::max_element(step_length.begin(), step_length.end()) + 1;
        for (auto i = 0; i < max_ele; i++) {
            auto forward_times = std::count(step_length.begin(), step_length.end(), i);
            printf("转发次数为%d的频数: %ld\n", i, forward_times);
        }
        printf("频数总和: %f\n", std::pow(GPU_NUM, 2));
    }


    void GenerateDefaultRoute::generate_confirm_and_save_specific_default_route() {

        auto route_table_ptr = generate_specific_default_route();   //生成指定维度的路由表
        confirm_route_table(*route_table_ptr);  //验证路由表的准确性

        std::string route_file_name = "route_default";
        for (auto const &e: dim) {
            route_file_name += ("_" + std::to_string(e));
        }
        auto route_able_file_save_path = BaseInfo::getInstance()->route_dir_path + "/" + route_file_name;
        save_data_to_file_test(route_able_file_save_path, *route_table_ptr);

        std::vector<std::vector<unsigned >> route;
        load_data_from_file_test(route_able_file_save_path, route);
    }

    void GenerateDefaultRoute::generate_confirm_and_save_specific_default_route_save_txt() {

        auto route_table_ptr = generate_specific_default_route();   //生成指定维度的路由表
        confirm_route_table(*route_table_ptr);  //验证路由表的准确性

        std::string route_file_name = "route_default";
        for (auto const &e: dim) {
            route_file_name += ("_" + std::to_string(e));
        }
        route_file_name += ".txt";
        std::cout << route_file_name << std::endl;
        auto route_able_file_save_path = BaseInfo::getInstance()->route_dir_path + "/" + route_file_name;

        save_txt_route_file(route_able_file_save_path, route_table_ptr, dim);

    }
}























