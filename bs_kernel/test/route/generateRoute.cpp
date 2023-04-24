//
// Created by 王明龙 on 2022/12/5.
//
#include "route/GenerateDefaultRoute.hpp"
#include "utils/TimePrint.hpp"


int main() {

    int a = 2;
    std::cout << (a << 2);

    dtb::TimePrint tp;
    std::vector<unsigned int> dim{50, 40};
    dtb::GenerateDefaultRoute gdr(dim);
//    gdr.generate_specific_default_route();
    gdr.generate_confirm_and_save_specific_default_route();
    tp.print_time();
    return 0;
}


