#pragma once

#include <mpi.h>
#include "LoadData.hpp"

namespace dtb {
    class MpiManage {
    public:
        MpiManage(int argc, char **argv);

        int getProSize() const;

        int getProRank() const;

        virtual void show_basic_information();

    protected:
        int pro_size;  //通信组的进程个数
        int pro_rank;  //本进程编号
        int master_rank;  //主进程编号
    };

    MpiManage::MpiManage(int argc, char **argv) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &pro_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &pro_size);
        master_rank = pro_size - 1;
    }

    int MpiManage::getProSize() const {
        return pro_size;
    }

    int MpiManage::getProRank() const {
        return pro_rank;
    }

    void MpiManage::show_basic_information() {
        if (pro_rank == master_rank) {     //主进程打印读取的数据信息
            dtb::LoadData::getLoadDataInstance()->show_basic_information();
            std::cout << "process sizes: " << pro_size << std::endl;
        }
    }
}
