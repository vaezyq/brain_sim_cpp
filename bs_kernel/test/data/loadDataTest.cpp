//
// Created by 22126 on 2022/12/5.
//
#include "../../inc/data/LoadData.hpp"
#include <iostream>
#include <algorithm>

using namespace std;


int main() {

    auto ptr = dtb::LoadData::getLoadDataInstance();


    ptr->show_basic_information();
    string degree_file_path = dtb::BaseInfo::getInstance()->conn_dir_path + "/" + "degree";


    string size_file_path = dtb::BaseInfo::getInstance()->conn_dir_path + "/" + "size";


    cout << "print size: " << endl;
    for (int i = 0; i < 100; ++i) {
        cout << ptr->getSizeTable()[i] << " ";
    }
    cout << endl;
    cout << "size sum: " << endl;
    cout << accumulate(ptr->getSizeTable().begin(), ptr->getSizeTable().end(), 0.0) << endl;


    cout << "degree : " << endl;
    for (int i = 0; i < 100; ++i) {
        cout << ptr->getDegreeTable()[i] << " ";
    }
    cout << endl;
    cout << "degree sum" << endl;
    cout << accumulate(ptr->getDegreeTable().begin(), ptr->getDegreeTable().end(), 0.0) << endl;


    cout << "show " << dtb::GPU_NUM << " gpu map: " << endl;
    for (auto &pop_idx_pre: ptr->getMapTable()[dtb::GPU_NUM - 1]) {
        cout << pop_idx_pre.first << " " << pop_idx_pre.second << " , ";
    }
    cout << endl;


    cout << "show " << dtb::GPU_NUM << " gpu route: " << endl;
    for_each(ptr->getRouteTable()[dtb::GPU_NUM - 1].begin(), ptr->getRouteTable()[dtb::GPU_NUM - 1].end(),
             [](unsigned a) { cout << a << " "; });
    cout << endl;


    return 0;
}
