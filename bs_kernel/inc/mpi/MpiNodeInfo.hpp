/*******************************************************************************
 * @file
 * @brief  提供对mpi进程的额外信息处理，包括节点对应的物理编号等
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/


#pragma once

#include <mpi.h>
#include <string>
#include <iostream>
#include "MpiManage.hpp"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <cstdlib>


using namespace std;

#define get_first_name(node_name)  (node_name.substr(0, 1))

#define get_first_seq(node_name)  (node_name.substr(1, 2))

#define get_sec_name(node_name)  node_name.substr(3, 1)

#define get_sec_seq(node_name) node_name.substr(4, 1)

#define get_third_name(node_name)  node_name.substr(5, 1)

#define get_third_seq(node_name)  node_name.substr(7, 2)

#define is_in_same_node(send_idx, recv_idx)  (send_idx / 4 == recv_idx / 4)


class MpiNodeInfo : public dtb::MpiManage {

public:

    /*!
     * 构造函数，传入MPI参数
     * @param argc
     * @param argv
     */
    MpiNodeInfo(int argc, char **argv) : dtb::MpiManage(argc, argv) {}

    /*!
     * 构造进程编号与节点编号之间的映射关系，注意一个节点可以有多个进程
     */
    void init_host_rank_map();


    /*！
     * 初始化第一级分组，后续可以添加更细致的分组。例如对于e12r2n06, 会得到 first_level_node["e"]["12"] = {"e12r2n06",...}
     * 因此第一级完全一致的会在一个分组内
     */
    void init_first_level_node();

    /*!
     * 调用初始化函数开始初始化，辅进程负责发送节点信息到主进程，主进程负责后续处理
     */
    void start_init_node_info();


private:

    void print_first_level_node_res();


protected:


    std::unordered_multimap<std::string, int> host_to_rank;

    std::unordered_multimap<int, std::string> rank_to_host;

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string> >> first_level_node;
};

void MpiNodeInfo::init_host_rank_map() {
    if (pro_rank == master_rank) {    //主进程

        host_to_rank.insert({host_name, pro_rank});
        rank_to_host.insert({pro_rank, host_name});
        std::string s;
        char p_name_tem[MPI_MAX_PROCESSOR_NAME];
        for (int i = 0; i < master_rank; ++i) {
            MPI_Status status;
            MPI_Recv(p_name_tem, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
            host_to_rank.insert({p_name_tem, i});
            rank_to_host.insert({i, p_name_tem});
        }
        MPI_Barrier(MPI_COMM_WORLD);
    } else {    //辅助进程

//            cout << host_name.size() << endl;
        MPI_Send(host_name.c_str(), static_cast<int>(host_name.size()) + 1, MPI_CHAR, master_rank, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void MpiNodeInfo::init_first_level_node() {
    for (auto &kv: host_to_rank) {
        first_level_node[get_first_name(kv.first)][get_first_seq(kv.first)].push_back(kv.first);
    }
}

void MpiNodeInfo::start_init_node_info() {
    init_host_rank_map();
    if (pro_rank == master_rank) {
        init_first_level_node();
        print_first_level_node_res();
    }
}


void MpiNodeInfo::print_first_level_node_res() {
    for (auto &um: first_level_node) {
        std::cout << um.first << " ";
        for (auto &kv: um.second) {
            std::cout << kv.first << " ";
            for (auto &e: kv.second) {
                std::cout << e << " ";
            }
            std::cout << kv.second.size() << std::endl;
            std::cout << std::endl;
        }
    }
}








