//
// Created by 22126 on 2022/12/6.
//

#include "../code/SimulationHighDimTraffic.hpp"
#include <iostream>
#include <mpi.h>


using namespace std;

int main(int argc, char **argv) {

    vector<double> vec(dtb::GPU_NUM * dtb::GPU_NUM);


//    dtb::SimulationOneDimTraffic sim;
    dtb::LoadData::get_traffic_table(vec);
    cout << vec.size() << endl;
//    cout << traffic_table[0].size() << endl;
    for (int i = 0; i < dtb::GPU_NUM; ++i) {
        cout << vec[i] << endl;
    }

//    dtb::SimulationHighDimTraffic sim;
////    sim.compute_simulation_traffic(argc, argv);
////    return 0;
////
//    double res = 0.0;
////    std::chrono::time_point<std::chrono::system_clock> start, end;
////
////    start = std::chrono::system_clock::now();
//////    double traffic=sim.sim_traffic_between_two_gpu(0, 1999);
//////    cout << 2000 << " " << traffic<< endl;
//    for (int i = 0; i < dtb::GPU_NUM; ++i) {
//        double traffic = sim.sim_traffic_between_two_gpu(100, i);
//        cout << i << " " << traffic << endl;
////        cout<<
//        res += traffic;
//    }
//
//
//    cout << res << endl;
//
//
//    end = std::chrono::system_clock::now();
//
//    std::chrono::duration<double> elapsed_seconds = end - start;
//    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
//    std::cout << "finished computation at " << std::ctime(&end_time)
//              << "elapsed time: " << elapsed_seconds.count() << "s\n";


}