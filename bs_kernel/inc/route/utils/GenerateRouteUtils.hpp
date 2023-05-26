/*******************************************************************************
 * @file
 * @brief  生成默认路由表的辅助函数
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/

#pragma once

#include "../../data/BaseInfo.hpp"
#include <vector>
#include <iostream>
#include "../../utils/AssertUtils.hpp"
#include <cmath>

namespace dtb {

    template<typename RouteTable>
    void confirm_route_table(const RouteTable &route_table) {
        std::vector<gpu_size_type> step_length(GPU_NUM * GPU_NUM, 0);

        std::cout << "Begin to confirm route table..." << std::endl;
        for (gpu_size_type src = 0; src < GPU_NUM; ++src) {
            for (gpu_size_type dst = 0; dst < GPU_NUM; ++dst) {
                unsigned temp_src = src;
                while ((route_table)[temp_src][dst] != temp_src) {
                    temp_src = (route_table)[temp_src][dst];
                    step_length[src * GPU_NUM + dst] += 1;
                    auto error = assert_expression("Too many forwards", step_length[src * GPU_NUM + dst] < 10);

                    if (error.has_value()) {
                        std::cout << error.value();
                        exit(1);
                    }
                }
            }
        }
        auto max_ele = *std::max_element(step_length.begin(), step_length.end()) + 1;
        gpu_size_type forward_sum{0};
        for (auto i = 0; i < max_ele; i++) {

#ifndef NDEBUG
            auto forward_times = std::count(step_length.begin(), step_length.end(), i);
            printf("转发次数为%d的频数: %ld\n", i, forward_times);
#endif
            forward_sum += std::count(step_length.begin(), step_length.end(), i);
        }
#ifndef NDEBUG
        printf("频数总和: %f\n", std::pow(GPU_NUM, 2));
#endif
        auto error = assert_expression("route table load failed", forward_sum == pow(GPU_NUM, 2));
        if (error.has_value()) {
            std::cout << error.value();
            exit(1);
        }
    }
}