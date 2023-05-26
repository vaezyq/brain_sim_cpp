//
// Created by 22126 on 2022/12/6.
//

//#include "../code/SimulationHighDimTraffic.hpp"

#include <iostream>
//#include <mpi.h>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
//#include "traffic/utils/SimulationTrafficUtils.hpp"
#include "../../inc/traffic/utils/SimulationTrafficUtils.hpp"
#include "../../inc/traffic/SimulationHighDimTraffic.hpp"
#include "../../inc/data/utils/ProcessFileDataUtils.hpp"

using namespace std;

int main(int argc, char **argv) {


    int flag = 4;


    if (flag == 1) {    //测试计算gpu to gpu的流量是否与真实模拟traffic.txt符合
        std::vector<std::vector<double> > traffic_res(dtb::GPU_NUM, std::vector<double>(dtb::GPU_NUM, 0));

//        double res = 0;
        dtb::SimulationTrafficUtils stu;
        for (int i = 0; i < 2000; ++i) {
            double traffic = stu.sim_traffic_between_two_gpu(0, i);
//            res += traffic;
            cout << i << " " << static_cast<unsigned long long> (traffic) << " ";
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
//        dtb::SimulationOneDimTraffic sod(argc, argv);
//
//        sod.show_basic_information();
////        sod.mpi_comm_test();
//        sod.compute_simulation_traffic();
    } else if (flag == 4) {  // 验证通过，注意路由表不要是错误的
        dtb::SimulationHighDimTraffic shd(argc, argv);

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


}