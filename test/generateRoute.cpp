//
// Created by 王明龙 on 2022/12/5.
//
#include "../code/GenerateDefaultRoute.hpp"
#include "../code/TimePrint.hpp"

//todo 生成路由测试通过，但目前默认路由可以通过array管理，而不需要vector。同时若后续补充不规则路由则需要vector
// 可以把vector替换成array，但是目前替换后会出现内存溢出的问题(待解决)
// 以及这些函数的返回值是否需要使用unique_ptr统一(待解决),目前生成的40*50的存在问题

int main() {
    dtb::TimePrint tp;
    std::vector<unsigned int> dim{50,40};
    dtb::GenerateDefaultRoute gdr(dim);
//    gdr.generate_specific_default_route();
    gdr.generate_confirm_and_save_specific_default_route();
    tp.print_time();

}


