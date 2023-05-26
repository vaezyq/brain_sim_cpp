//
// Created by 王明龙 on 2023/5/21.
//
#include "../../inc/mpi/MpiNodeInfo.hpp"


int main(int argc, char **argv) {

    MpiNodeInfo mpiNodeInfo(argc, argv);
    mpiNodeInfo.start_init_node_info();
    return 0;
}