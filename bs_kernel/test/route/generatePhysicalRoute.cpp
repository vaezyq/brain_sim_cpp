//
// Created by 王明龙 on 2023/5/22.
//
#include "../../inc/route/GenerateRouteByPhysicalTopology.hpp"


int main(int argc, char **argv) {
    dtb::GenerateRouteByPhysicalTopology generateRouteByPhysicalTopology(argc, argv);


    generateRouteByPhysicalTopology.start_generate_physical_topology();

    return 0;
}