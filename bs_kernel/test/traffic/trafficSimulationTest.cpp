//
// Created by 22126 on 2022/12/6.
//

//#include "../code/SimulationHighDimTraffic.hpp"

#include <iostream>
#include <mpi.h>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include "traffic/SimulationOneDimTraffic.hpp"
#include "traffic/SimulationHighDimTraffic.hpp"
#include "data/utils/ProcessFileDataUtils.hpp"

using namespace std;

int main(int argc, char **argv) {


    int flag = 5;


    if (flag == 1) {    //测试计算gpu to gpu的流量是否与真实模拟traffic.txt符合
        std::vector<std::vector<double> > traffic_res(dtb::GPU_NUM, std::vector<double>(dtb::GPU_NUM, 0));

        std::string traffic_file_name = "traffic_table_base_dcu_out_in_1_dimmap_2000_sequential_cortical_v2.txt";
        auto base_info_ptr = dtb::BaseInfo::getInstance();
        base_info_ptr->traffic_read_write_path + "/" + traffic_file_name;

        dtb::load_one_dim_traffic_two_dim_result(traffic_file_name, traffic_res);

//        double res = 0;
        dtb::SimulationTrafficUtils stu;
        for (int i = 0; i < 2000; ++i) {
            double traffic = stu.sim_traffic_between_two_gpu(0, i);
//            res += traffic;
            cout << i << " " << static_cast<unsigned long long> (traffic) << " " << traffic_res[0][i] << endl;
        }
    } else if (flag == 2) {
        dtb::SimulationTrafficUtils stu;

        std::vector<unsigned> recv_lists(dtb::GPU_NUM, 0);
        std::generate(recv_lists.begin(), recv_lists.end(), [i = 0]()mutable { return i++; });

        std::array<dtb::traffic_size_type, dtb::GPU_NUM << 2> output_input_traffic{};

        auto const &forward_idx = stu.get_list_send_by_route_table(0, recv_lists);
        for (auto i = 0; i != 1; ++i) {
            stu.simulate_2_dim_input_output_traffic_per_gpu_no_recursive(i, output_input_traffic);
        }
        for (int i = 0; i != 2000; i++) {
            if (output_input_traffic[4 * i] != 0 || output_input_traffic[4 * i + 1] != 0) {
                cout << i << " " << output_input_traffic[4 * i] << " "
                     << output_input_traffic[4 * i + 1] << endl;
            }
        }

        cout << "input" << endl;
        for (int i = 0; i != 2000; i++) {
            if (output_input_traffic[4 * i + 2] != 0 || output_input_traffic[4 * i + 3] != 0) {
                cout << i << " " << output_input_traffic[4 * i + 2] << " "
                     << output_input_traffic[4 * i + 3] << endl;
            }
        }
    } else if (flag == 3) {
        dtb::SimulationOneDimTraffic sod(argc, argv);

        sod.show_basic_information();
//        sod.mpi_comm_test();
        sod.compute_simulation_traffic();
    } else if (flag == 4) {  // 验证通过，注意路由表不要是错误的
        dtb::SimulationHighDimTraffic shd(argc, argv);
        shd.show_basic_information();
        shd.compute_simulation_traffic();
    } else if (flag == 5) {

        dtb::SimulationTrafficUtils stu;

        std::array<dtb::traffic_size_type, dtb::GPU_NUM << 2> output_input_traffic{};

        for (auto i = 1997; i != 1998; ++i) {
            dtb::TimePrint tp;
            stu.simulate_2_dim_input_output_traffic_per_gpu_no_recursive_thread_version(i, output_input_traffic);

//            stu.simulate_2_dim_input_output_traffic_per_gpu_no_recursive(i, output_input_traffic);
            tp.print_time();
        }
        for (int i = 0; i != 2000; i++) {
            if (output_input_traffic[4 * i] != 0 || output_input_traffic[4 * i + 1] != 0) {
                cout << i << " " << output_input_traffic[4 * i] << " "
                     << output_input_traffic[4 * i + 1] << endl;
            }
        }

        cout << "input" << endl;
        for (int i = 0; i != 2000; i++) {
            if (output_input_traffic[4 * i + 2] != 0 || output_input_traffic[4 * i + 3] != 0) {
                cout << i << " " << output_input_traffic[4 * i + 2] << " "
                     << output_input_traffic[4 * i + 3] << endl;
            }
        }


    }


//    else if (flag == 4) {
//        dtb::SimulationHighDimTraffic shd(argc, argv);
//        sod.show_basic_information();
//        sod.compute_simulation_traffic();
//    }






//    sim.compute_simulation_traffic(argc, argv);
//    auto ptr = dtb::LoadData::getLoadDataInstance();
//
//    for (int i = 0; i < dtb::GPU_NUM; ++i) {
//        cout << ptr->getRouteTable()[0][i] << " ";
//    }



//    int index = 0;
//    for (auto &e: *forward_idx) {
//        cout << "e.first: " << e.first << " ";
//        cout << endl;
//        for_each(e.second.begin(), e.second.end(), [](int i) { cout << i << " "; });
//        cout << endl;
//        index++;
//    }
//    cout << index;

//
////
//
//    cout << sim.sim_traffic_between_gpu_group(1, {1, 2, 3, 4});
//    auto const &forward_idx = sim.get_list_send_by_route_table(0, recv_lists);
//    dtb::TimePrint tp;
//


//    for_each(output_input_traffic.begin(), output_input_traffic.end(), [](int i) {
//        cout << i << endl;
//    });


//
//    auto ptr = dtb::LoadData::getLoadDataInstance();
////    auto map_table= ptr->getMapTable();
//    for (auto &e: ptr->getMapTable()[100]) {
//        cout << e.first << " " << e.second << endl;
//    }

//
//    std::vector<std::vector<double> > traffic_res(dtb::GPU_NUM, std::vector<double>(dtb::GPU_NUM, 0));
////////
//    dtb::LoadData::load_one_dim_traffic_result(traffic_res);
//
//    double res = 0;
//
//    dtb::TimePrint tp;
//    for (int i = 0; i < 1; ++i) {
//
//        double traffic = sim.sim_traffic_between_two_gpu(100, i);
//        res += traffic;
////        cout << i << " " << traffic << " " << traffic_res[100][i] << endl;
////        cout<<
//    }
//    cout << "traffic: " << res << endl;
//    cout << accumulate(traffic_res[100].begin(), traffic_res[100].end(), 0.0) << endl;
//    tp.print_time();


//    for (int i = 10000; i <= 30000; i += 1000) {
//        cout << i << " " << dtb::sample(i, 30000) << endl;
//    }





//    int N = 1000;
//    vector<int> vec(N, 0);
//    std::generate(vec.begin(), vec.end(), [n = 0]() mutable { return n++; });
//
//    vector<int> out;
//    int sample_times = 10;
//    std::sample(vec.begin(), vec.end(), std::back_inserter(out),
//                sample_times, std::mt19937{std::random_device{}()});
//    std::for_each(out.begin(), out.end(), [](int i) { cout << i << " "; });


//
//
////
//////    dtb::SimulationOneDimTraffic sim;
//////    dtb::LoadData::get_traffic_table(vec);
//////    cout << vec.size() << endl;
//////    cout << traffic_table[0].size() << endl;
////



//    std::vector<std::vector<double> > traffic_res(dtb::GPU_NUM, std::vector<double>(dtb::GPU_NUM, 0));
//////
//    dtb::LoadData::load_one_dim_traffic_result(traffic_res);

//    double res = 0;
//    dtb::SimulationHighDimTraffic sim;
//    for (int i = 1949; i < 1950; ++i) {
//
//        double traffic = sim.sim_traffic_between_two_gpu(100, i);
//        res += traffic;
//        cout << i << " " << traffic << " " << traffic_res[100][i] << endl;
////        cout<<
//    }
//
//    cout << "traffic: " << res << endl;
//    cout << accumulate(traffic_res[100].begin(), traffic_res[100].end(), 0.0) << endl;

//////    sim.compute_simulation_traffic(argc, argv);
//////    return 0;
//////
////    double res = 0.0;
//////    std::chrono::time_point<std::chrono::system_clock> start, end;
//////
//////    start = std::chrono::system_clock::now();
////////    double traffic=sim.sim_traffic_between_two_gpu(0, 1999);
////////    cout << 2000 << " " << traffic<< endl;

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