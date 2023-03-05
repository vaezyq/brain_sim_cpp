/*******************************************************************************
 * @file
 * @brief  提供可变参数表达式断言
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <string_view>


namespace dtb {

    /*!
     * 自定义的assert函数，支持判断多个表达式
     * @tparam Expressions  变长表达式类型
     * @param s  字符串
     * @param expressions 多个表达式
     * @return  是否有错误表达式
     */
    template<typename... Expressions>
    std::optional<std::string> assert_expression(std::string_view s, const Expressions &... expressions) {
        std::vector<int> error;
        int cnt = 0;
        std::initializer_list<bool>{
                static_cast<bool>((++cnt, !static_cast<bool>(expressions) ? ((error.push_back(cnt)), 0)
                                                                          : 0), expressions)...};
        std::optional<std::string> ans;


        std::string error_info;
        if (!error.empty()) {
            for_each(error.begin(), error.end(), [&ans, &error_info](int a) {
                error_info.append(std::to_string(a));
                error_info.append(" ");
            });
            error_info.append(s);
            ans = error_info;
        }
        return ans;
    }

}

