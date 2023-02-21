//
// Created by 王明龙 on 2022/12/5.
//
#include "../code/GenerateDefaultRoute.hpp"
#include "../code/TimePrint.hpp"

//todo 生成路由测试通过，但目前默认路由可以通过array管理，而不需要vector。同时若后续补充不规则路由则需要vector
// 可以把vector替换成array，但是目前替换后会出现内存溢出的问题(array默认是生成在栈区的，如果开辟2000*2000大小的路由表会栈溢出)
// 以及这些函数的返回值是否需要使用unique_ptr统一(待解决)

int main() {
    dtb::TimePrint tp;
    std::vector<unsigned int> dim{10, 200};
    dtb::GenerateDefaultRoute gdr(dim);
//    gdr.generate_specific_default_route();
    gdr.generate_confirm_and_save_specific_default_route();
    tp.print_time();
    return 0;
}


