//
// Created by 22126 on 2022/12/6.
//


//#include <mpich-x86_64//mpi.h>
//
//#include <iostream>

#define MAX_LEN 100

using namespace std;


#include "../../inc/mpi/MpiNodeInfo.hpp"


int main(int argc, char **argv) {


    while (int i=1;i<10){
        ++i;
    }
//    MpiNodeInfo mpiNodeInfo(argc, argv);
//    mpiNodeInfo.start_init();
//    return 0;
}


//int main(int argc, char **argv) {
//    MPI_Init(&argc, &argv);
//
//    int rank, num;
//
//    int Version, SubVersion;
//    int RankID, ProcessNum, Length;
//    char ProcessName[MPI_MAX_PROCESSOR_NAME];
//
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &num);
//
//    MPI_Get_processor_name(ProcessName, &Length);
//    MPI_Get_version(&Version, &SubVersion);
//
//    std::cout << "Hello!, I am " << RankID << " of " << ProcessNum << " on " << ProcessName << std::endl;
//    std::cout << "My version is " << Version << "." << SubVersion << std::endl;
//
//    if (rank == num - 1) {    //主进程
//        string message = "test";
//        for (int i = 0; i < num - 1; ++i) {
//            MPI_Send(message.c_str(), message.size(), MPI_CHAR, i, 0, MPI_COMM_WORLD);
//        }
//    } else {
//        char message[MAX_LEN];
//        MPI_Status status;
//        MPI_Recv(message, MAX_LEN, MPI_CHAR, num - 1, 0, MPI_COMM_WORLD, &status);
//        ::printf("%i received %s \n", rank, message);
//    }
//}