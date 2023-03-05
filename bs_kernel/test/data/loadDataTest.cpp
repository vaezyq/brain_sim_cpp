//
// Created by 22126 on 2022/12/5.
//
#include "data/LoadData.hpp"
#include <iostream>
#include "utils/AssertUtils.hpp"

using namespace std;

static void get_host_name(char *hostname, int maxlen) {
    gethostname(hostname, maxlen);
    for (int i = 0; i < maxlen; i++) {
        if (hostname[i] == '.') {
            hostname[i] = '\0';
            return;
        }
    }
}

static uint64_t get_host_hash(const char *string) {
    // Based on DJB2, result = result * 33 + char
    uint64_t result = 5381;
    for (int c = 0; string[c] != '\0'; c++) {
        result = ((result << 5) + result) + string[c];
    }

    return result;
}

int main() {
    char hostname[1024];
    get_host_name(hostname, sizeof(hostname));
    cout << hostname << endl;

    cout << get_host_hash("e04r1n03") << endl;
    cout << get_host_hash("e04r1n03") << endl;

//    dtb::TimePrint tp;
//    auto ptr = dtb::LoadData::getLoadDataInstance();
//    tp.print_time();


//    string degree_file_path = dtb::BaseInfo::getInstance()->conn_dir_path + "/" + "degree";
//
//
//    string size_file_path = dtb::BaseInfo::getInstance()->conn_dir_path + "/" + "size";
//    dtb::save_data_to_binary_file(degree_file_path, ptr->getDegreeTable());

//
//    dtb::save_one_dim_array_data_to_binary_file(degree_file_path, ptr->getDegreeTable());
//    dtb::save_one_dim_array_data_to_binary_file(size_file_path, ptr->getSizeTable());

//    cout << "show " << dtb::GPU_NUM << " gpu map: " << endl;
//    for (auto &pop_idx_pre: ptr->getMapTable()[dtb::GPU_NUM - 1]) {
//        cout << pop_idx_pre.first << " " << pop_idx_pre.second << " , ";
//    }
//    cout << endl;
//
//
//    cout << "show " << dtb::GPU_NUM << " gpu route: " << endl;
//    for_each(ptr->getRouteTable()[dtb::GPU_NUM - 1].begin(), ptr->getRouteTable()[dtb::GPU_NUM - 1].end(),
//             [](unsigned a) { cout << a << " "; });
//    cout << endl;
    return 0;
}
