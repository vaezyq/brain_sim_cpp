//
// Created by 王明龙 on 2022/12/5. 生成默认路由表
//


#include <vector>
#include <algorithm>
#include <numeric>
#include <memory>
#include <iostream>
#include <fstream>
#include "BaseInfo.hpp"
#include <cassert>


namespace dtb {

    class GenerateDefaultRoute {  //生成默认路由表

    public:
        /*!
         * 生成指定维度的路由表的构造函数
         * @param dim 需要生成的维度，生成1800卡2维的，即可以填{45,40}
         */
        explicit GenerateDefaultRoute(const std::vector<unsigned int> &dim);

        /*!
         * 生成路由表的主函数
         * @return
         */
        std::unique_ptr<std::vector<std::vector<unsigned >>> generate_specific_default_route();

        /*!
         * 验证生成的路由表的正确性
         */
        static void confirm_route_table(std::vector<std::vector<unsigned >> const &);

        /*!
         *主调用函数，用于生成路由表、验证路由表准确性，并保存路由表到txt文件
         */
        void generate_confirm_and_save_specific_default_route();

    private:

        /*!
         * 得到在指定维度dimensions下的每个点的坐标
         * @return 返回一个unique_ptr，其包含在指定维度dim下，N个节点的坐标
         */
        std::unique_ptr<std::vector<std::vector<unsigned int >>> get_rank_coordinate();


        /*!
         * get_rank_coordinate的反函数，指定卡坐标和维度计算其编号
         * @param rank_cor 卡坐标
         * @return 卡编号
         */
        unsigned int get_rank_num(std::vector<unsigned int> const &rank_cor);

        std::vector<unsigned int> dim;   //存储维度

        unsigned int dim_len;   //维度的个数

    };

    GenerateDefaultRoute::GenerateDefaultRoute(const std::vector<unsigned int> &dim) : dim(dim) {
        int N = std::accumulate(dim.begin(), dim.end(), 1, [](int a, int b) { return a * b; });   //节点数目为累乘结果
        assert(N == GPU_NUM);  //判断节点数目是否等于模拟的卡数
        dim_len = dim.size();
    }

    std::unique_ptr<std::vector<std::vector<unsigned int >>> GenerateDefaultRoute::get_rank_coordinate() {
        auto rank_coordinate_ptr = std::make_unique<std::vector<std::vector<unsigned int>>>(
                std::vector<std::vector<unsigned int> >(GPU_NUM, std::vector<unsigned int>(dim_len, 0))
        );  //每个节点在N维情况下的坐标
        for (unsigned int idx = 0; idx != GPU_NUM; ++idx) {
            unsigned int num = idx;
            for (int j = static_cast<int>(dim_len) - 1; j != -1; --j) {
                (*rank_coordinate_ptr)[idx][j] = (num % dim[j]);
                num /= dim[j];
            }
        }
        return rank_coordinate_ptr;
    }


    unsigned int GenerateDefaultRoute::get_rank_num(const std::vector<unsigned int> &rank_cor) {
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
        auto route_table_ptr = std::make_unique<std::vector<std::vector<unsigned int>>>(
                std::vector<std::vector<unsigned int> >(GPU_NUM,
                                                        std::vector<unsigned int>(GPU_NUM, 0)));
        for (int in_rank = 0; in_rank != GPU_NUM; ++in_rank) {
            std::cout << in_rank << std::endl;

            for (int out_rank = 0; out_rank != GPU_NUM; ++out_rank) {
                if (out_rank == in_rank) {
                    (*route_table_ptr)[in_rank][out_rank] = in_rank;
                    continue;
                }
                std::vector<unsigned int> in_rank_cor_temp(dim_len, 0);
                std::copy((*rank_coordinate_ptr)[in_rank].begin(), (*rank_coordinate_ptr)[in_rank].end(),
                          in_rank_cor_temp.begin());

                unsigned int index = 0;
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

    void GenerateDefaultRoute::confirm_route_table(std::vector<std::vector<unsigned >> const &route_table) {
        std::vector<unsigned> step_length(GPU_NUM * GPU_NUM, 0);

//        std::array<std::array<unsigned, GPU_NUM>, GPU_NUM> step_length;
        std::cout << "Begin to confirm route table..." << std::endl;
        for (int src = 0; src < GPU_NUM; ++src) {
            for (int dst = 0; dst < GPU_NUM; ++dst) {
                unsigned temp_src = src;
                while ((route_table)[temp_src][dst] != temp_src) {
                    temp_src = (route_table)[temp_src][dst];
                    step_length[src * GPU_NUM + dst] += 1;
                    assert(step_length[src * GPU_NUM + dst] < 10);
                }
            }
            if (src / 1000 == 0) {
                printf("%d / %d \n", src, GPU_NUM);
            }
        }
        auto max_ele = *std::max_element(step_length.begin(), step_length.end()) + 1;
        for (int i = 0; i < max_ele; i++) {

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
        route_file_name += ".txt";
        std::cout << route_file_name << std::endl;
        std::ofstream outFile;
        //打开文件

        outFile.open(BaseInfo::getInstance()->route_path + "/" + route_file_name, std::ios::out);

        if (!outFile.is_open()) {
            std::cout << "open error!" << std::endl;
            return;
        }
        int N = std::accumulate(dim.begin(), dim.end(), 1, [](int a, int b) { return a * b; });   //节点数目为累乘结果
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                outFile << (*route_table_ptr)[i][j] << " ";
            }
            outFile << std::endl;
        }
        //关闭文件
        outFile.close();
    }
}























