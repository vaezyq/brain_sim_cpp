//
// Created by 22126 on 2022/12/5.
//
#include "../code/LoadData.hpp"
#include <iostream>
#include "../code/TimePrint.hpp"

using namespace std;

int main() {
    dtb::TimePrint tp;
    auto ptr = dtb::LoadData::getLoadDataInstance();
    tp.print_time();
    ptr->show_basic_information();

    cout << "show " << dtb::GPU_NUM << " gpu map: " << endl;
    for (auto &pop_idx_pre: ptr->getMapTable()[dtb::GPU_NUM-1]) {
        cout << pop_idx_pre.first << " " << pop_idx_pre.second << " , ";
    }
    cout << endl;
    cout << "show " << dtb::GPU_NUM << " gpu route: " << endl;
    for_each(ptr->getRouteTable()[dtb::GPU_NUM-1].begin(), ptr->getRouteTable()[dtb::GPU_NUM-1].end(), [](unsigned a) { cout << a << " "; });
    cout << endl;
    return 0;
}
