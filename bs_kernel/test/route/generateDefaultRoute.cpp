//
// Created by 王明龙 on 2022/12/5.
//
#include "../../inc/route/GenerateDefaultRoute.hpp"
#include "../../inc/utils/TimePrint.hpp"


int main() {

    dtb::TimePrint tp;
    std::vector<unsigned int> dim{40, 25};
    dtb::GenerateDefaultRoute gdr(dim);
//    gdr.generate_specific_default_route();
    gdr.generate_confirm_and_save_specific_default_route();
    tp.print_time();
    return 0;
}


