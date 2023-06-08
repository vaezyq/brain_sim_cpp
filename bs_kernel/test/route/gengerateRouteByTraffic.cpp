//
// Created by 王明龙 on 2023/5/27.
//

#include "../../inc/route/GenerateRouteByTraffic.hpp"
#include "../../inc/data/LoadData.hpp"
#include "../../inc/data/utils/ProcessFileDataUtils.hpp"
#include "../../inc/route/utils/GenerateRouteUtils.hpp"


using namespace dtb;
using namespace std;

int main() {

    std::array<std::array<gpu_size_type, GPU_NUM>, GPU_NUM> route_table{};


    auto load_data_ptr = LoadData::getLoadDataInstance();

    for (gpu_size_type i = 0; i < GPU_NUM; ++i) {
        for (gpu_size_type j = 0; j < GPU_NUM; ++j) {
            route_table[i][j] = load_data_ptr->getRouteTable()[i][j];
        }
    }
    std::array<traffic_size_type, GPU_NUM * 2 * 2> output_input_traffic{};

    std::string file_path = "/public/home/ssct005t/project/wml_istbi/brain_sim_cpp/tables/traffic_tables/traffic_1000/route_phy__traffic_map_1000_after_size_balance_random_without_invalid_index.txt";
    load_one_dim_data_from_txt_file(file_path, output_input_traffic);

    unsigned dimensions = 2;

    std::array<traffic_size_type, GPU_NUM> output_traffic{}, input_traffic{};

    for (size_t i = 0; i < GPU_NUM; ++i) {
        output_traffic[i] += output_input_traffic[(i << 2)];
        output_traffic[i] += output_input_traffic[(i << 2) + 1];

        input_traffic[i] += output_input_traffic[(i << 2) + dimensions];
        input_traffic[i] += output_input_traffic[(i << 2) + 1 + dimensions];
    }

    std::cout << "max output traffic: " << *std::max_element(output_traffic.begin(), output_traffic.end())
              << std::endl;

    std::cout << "min output traffic: " << *std::min_element(output_traffic.begin(), output_traffic.end())
              << std::endl;

    std::cout << "average output traffic: "
              << std::accumulate(output_traffic.begin(), output_traffic.end(), 0.0) / output_traffic.size()
              << std::endl;

    std::cout << "max input traffic: " << *std::max_element(input_traffic.begin(), input_traffic.end())
              << std::endl;

    std::cout << "min input traffic: " << *std::min_element(input_traffic.begin(), input_traffic.end())
              << std::endl;

    std::cout << "average input traffic: "
              << std::accumulate(output_traffic.begin(), output_traffic.end(), 0.0) / input_traffic.size()
              << std::endl;


//    for (int i = 0; i < GPU_NUM; ++i) {
//        for (int j = 0; j < GPU_NUM; ++j) {
//            std::cout << load_data_ptr->getRouteTable()[i][j] << " ";
//        }
//        cout << endl;
//    }

    std::array<traffic_size_type, GPU_NUM> traffic{};

    for (gpu_size_type i = 0; i < GPU_NUM; ++i) {
        traffic[i] = max(input_traffic[i], output_traffic[i]);
    }

    GenerateRouteByTraffic::change_route_by_traffic_v2(traffic, route_table);

    confirm_route_table(route_table);

    auto route_file_path = BaseInfo::getInstance()->route_dir_path + "/" + "changed_1000_route";

    save_two_dim_data_to_binary_file(route_file_path, route_table);

    dtb::save_two_dim_route_data_to_txt_file(route_file_path + ".txt", route_table,
                                             {static_cast<gpu_size_type >(GPU_NUM)});

    std::cout << "route table saved" << std::endl;

}