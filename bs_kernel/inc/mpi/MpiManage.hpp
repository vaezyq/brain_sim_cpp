/*******************************************************************************
 * @file
 * @brief  提供mpi的统一进程管理
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/


#pragma once

#include <mpi.h>
#include <string>
#include <iostream>
#include <vector>

namespace dtb {
    class MpiManage {
    public:
        /*!
         * mpi的构造函数，用于生成mpi的环境
         * @param argc
         * @param argv
         */
        MpiManage(int argc, char **argv);

        /*!
         * 得到本次运行的进程数目
         * @return 本次运行的进程数目
         */
        int getProSize() const;

        /*!
         * 得到本进程的编号
         * @return 本进程的编号
         */
        int getProRank() const;

        /*!
         * 得到本进程的主机名
         * @return
         */
        const std::string &get_process_host_name() const;

        std::vector<std::vector<std::string>> process_first_level_host();

        virtual void show_basic_information();


    protected:
        int pro_size;  //通信组的进程个数
        int pro_rank;  //本进程编号
        int master_rank;  //主进程编号
        std::string host_name;
        //进程所在主机名
    };


    MpiManage::MpiManage(int argc, char **argv) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &pro_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &pro_size);
        master_rank = pro_size - 1;
        int length{};   //保存返回的主机名字符串长度
        char p_name_tem[MPI_MAX_PROCESSOR_NAME];
        MPI_Get_processor_name(p_name_tem, &length);
        host_name = p_name_tem;
    }


    int MpiManage::getProSize() const {
        return pro_size;
    }

    int MpiManage::getProRank() const {
        return pro_rank;
    }

    void MpiManage::show_basic_information() {
        if (pro_rank == master_rank) {     //主进程打印读取的数据信息
            std::cout << "MPI environment initialization" << std::endl;
        }
    }

    const std::string &MpiManage::get_process_host_name() const {
        return host_name;
    }

    std::vector<std::vector<std::string>> MpiManage::process_first_level_host() {

        if (pro_rank == master_rank) {     //主进程打印读取的数据信息
            std::cout << "MPI environment initialization" << std::endl;
        } else {

        }


    }

}
