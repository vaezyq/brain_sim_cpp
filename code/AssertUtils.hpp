#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <string_view>


namespace dtb {

    template<typename... Expressions>
    std::optional<std::string> assert_load_data(std::string_view s, const Expressions &... expressions) {
        std::vector<int> error;
        int cnt = 0;
        std::initializer_list<bool>{
                static_cast<bool>((++cnt, !static_cast<bool>(expressions) ? ((error.push_back(cnt)), 0)
                                                                          : 0), expressions)...};
        std::optional<std::string> ans;

        for_each(error.begin(), error.end(), [&ans](int a) {
            ans->append(std::to_string(a));
            ans->append(" ");
        });
        if (!error.empty()) { ans->append(s); }
        return ans;
    }

}

