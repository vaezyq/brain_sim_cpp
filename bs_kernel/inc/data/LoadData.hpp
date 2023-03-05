/*******************************************************************************
 * @file
 * @brief  加载表数据的类
 * @details
 * @author
 * @date       2022-11-01
 * @version    V1.0
 *******************************************************************************/


#pragma once

#include <unordered_map>
#include <fstream>
#include <memory>
#include <string>
#include "Utils/Utils.hpp"
#include <cassert>
#include <array>
#include "Utils/AssertUtils.hpp"
#include "data/utils/ProcessFileDataUtils.hpp"
#include "route/utils/GenerateRouteUtils.hpp"



/*
 * todo: 加载数据的部分或许可以参考下述链接进行重构
 * 目前存储的为txt文件，因此在读取上依赖于stringstream的构造,当构造了很多stringstream时会严重降低读取速度
 * 目前读取全部文件耗时大概23s
 * https://stackoverflow.com/questions/71661178/faster-way-of-loading-big-stdvectorstdvectorfloat-from-file
 * https://stackoverflow.com/questions/54238956/fastest-way-to-read-a-vectordouble-from-file
 */


//todo: conn、map、degree的读取可以参考route写成二进制的存储与读取，会加快速度，目前读取全部文件需要23s附近
//conn等目前基于ifstream构造读取，ifstream构造过多时会读取很慢


namespace dtb {

    class LoadData {
    private:
        static std::shared_ptr<LoadData> instance_ptr;      //读取数据函数设计为单例,通过instance_ptr管理

        LoadData();

        static std::shared_ptr<BaseInfo> base_info_ptr;     //基本信息被设计为单例，这是基本信息类单例指针

        std::unordered_map<double, double> conn_dict_table;   //连接表(conn)数据,key-value形式

        std::array<double, POP_NUM> size_table; //体素大小数据

        std::array<gpu_size_type, POP_NUM> degree_table; //体素出度数据

        std::vector<std::unordered_map<gpu_size_type, double> > map_table;   //map表数据

        std::array<std::array<gpu_size_type, GPU_NUM>, GPU_NUM> default_route_table;     //默认路由表数据,后续可能会加入不规整路由表,这个是规整路由表


    public:
        /*!
         * 加载全部数据的总调用函数
         */
        void load_data();

        /*!
         * 验证加载的数据的正确性
         */
        void confirm_load_data();

        /*!
         * 本类应用了单例模式,此接口用于返回此类的唯一实例
         * @return 返回指向本类实例的shared_ptr指针
         */
        static std::shared_ptr<LoadData> getLoadDataInstance();

        /*!
         * 加载连接表数据
         */
        void load_conn();

        /*!
         * 和基本文件对比，用于验证连接表读取是否正确，是否存在精度损失(待完成)
         */
        void confirm_conn_table();


        /*!
         * 通过对所有拆分加和来验证map表是否完成
         */
        void confirm_map_table();

        /*!
         * 加载体素size
         */
        void load_size();


        /*!
         * 加载体素degree
         */
        void load_degree();

        /*!
         * 加载map表
         */
        void load_map();

        /*!
         * 加载路由表数据
         */
        void load_route();

        inline const std::unordered_map<double, double> &getConnDictTable() const;

        inline const std::array<double, POP_NUM> &getSizeTable() const;

        inline const std::array<gpu_size_type, POP_NUM> &getDegreeTable() const;

        inline std::vector<std::unordered_map<gpu_size_type, double>> &getMapTable();

        inline const std::array<std::array<gpu_size_type, GPU_NUM>, GPU_NUM> &getRouteTable() const;

        LoadData(const LoadData &) = delete;

        LoadData &operator=(const LoadData &) = delete;

        /*!
         * 输出加载后的数据信息
         */
        void show_basic_information();

    };

    void LoadData::load_data() {
        //todo:这里应加上验证：验证路由表是否全部走通，大小是否对。验证map表所有map加和是否完整
//        load_map();
//        load_conn();

        load_map(), load_conn(), load_size(), load_degree(), load_route();

//        confirm_load_data();  //验证读取结果是否正确，因为验证路由表很耗时，有时可以省略这一步
        show_basic_information();
    }


    std::shared_ptr<LoadData> LoadData::instance_ptr = nullptr;

    std::shared_ptr<BaseInfo>  LoadData::base_info_ptr = BaseInfo::getInstance();

    void LoadData::load_conn() {      //这里使用double，使用更小的精度或许可以减少内存占用

        std::string conn_dict_table_file_path = base_info_ptr->conn_dir_path + "/" + base_info_ptr->conn_file_name;

        dtb::load_conn_data_from_binary_file(conn_dict_table_file_path, conn_dict_table);
//        try {
//            //这里在4G的内存上运行会报错，内存不够
//            std::ifstream conn_dict_int;
//            std::string line;
//            conn_dict_int.open(conn_dict_table_file_path);
//            /*
//            * 这里参考的https://stackoverflow.com/questions/9670396/exception-handling-and-opening-a-file
//            * 和https://stackoverflow.com/questions/3629321/try-catch-block-for-c-file-io-errors-not-working
//            * 注意eofbit是不能加的，因为读取文件到末尾一定会出发这个异常
//            */
//            conn_dict_int.exceptions(std::ifstream::badbit);
//            double k{0.0}, v{0.0};
////            long idx = 0;
//            while (getline(conn_dict_int, line)) {           //conn表为一行两个double数据，分别是key value
////                idx++;
//                k = strtod(line.substr(0, 24).c_str(), nullptr);
//                v = strtod(line.substr(25, 24).c_str(), nullptr);
//                conn_dict_table.insert(std::make_pair(k, v));
////                conn_dict_table[k] = v;
////                std::cout.precision(19);
////            if (idx % 10000 == 0) {
////                std::cout << "k " << k << std::endl;
////                std::cout << "v " << v << std::endl;
////            }
//            }
//        } catch (std::ios_base::failure &e) {
//            std::cout << "load conn table file " << conn_dict_table_file_path << " failed," << e.what() << std::endl;
//            exit(1);
//        }
    }

    void LoadData::load_map() {

        std::string map_table_file_path = base_info_ptr->map_read_path + '/' + base_info_ptr->map_file_name;

        load_map_data_from_binary_file(map_table_file_path,map_table);

    }

    void LoadData::load_size() {


        load_one_dim_data_from_binary_file(base_info_ptr->conn_dir_path + "/" + base_info_ptr->size_file_name,
                                           size_table);


//        load_vector_data_from_txt(size_table, base_info_ptr->conn_dir_path + "/" + base_info_ptr->size_file_name);

//        load_vector_data(size_table, base_info_ptr->conn_dir_path + "/" + base_info_ptr->size_file_name);
    }

    void LoadData::load_degree() {


        load_one_dim_data_from_binary_file(base_info_ptr->conn_dir_path + "/" + base_info_ptr->degree_file_name,
                                           degree_table);

//        load_vector_data_from_txt(degree_table, base_info_ptr->conn_dir_path + "/" + base_info_ptr->degree_file_name);

//        load_vector_data(degree_table, base_info_ptr->conn_dir_path + "/" + base_info_ptr->degree_file_name);
    }

    void LoadData::load_route() {
        std::string route_table_file_path = base_info_ptr->route_dir_path + "/" + base_info_ptr->route_file_name;

        load_two_dim_data_from_binary_file(route_table_file_path, default_route_table);
    }

    const std::array<double, POP_NUM> &LoadData::getSizeTable() const {
        return size_table;
    }

    const std::array<gpu_size_type, POP_NUM> &LoadData::getDegreeTable() const {
        return degree_table;
    }


    const std::unordered_map<double, double> &LoadData::getConnDictTable() const {
        return conn_dict_table;
    }


    std::vector<std::unordered_map<gpu_size_type, double>> &LoadData::getMapTable() {
        return map_table;
    }

    const std::array<std::array<gpu_size_type, GPU_NUM>, GPU_NUM> &LoadData::getRouteTable() const {
        return default_route_table;
    }

    std::shared_ptr<LoadData> LoadData::getLoadDataInstance() {
        if (!instance_ptr) {   //若还没有初始化，即第一次加载数据
            instance_ptr = std::shared_ptr<LoadData>(new LoadData());   //构造对象并在构造函数内加载数据
        }
        return instance_ptr;
    }

    LoadData::LoadData() {
        load_data();
    }

    void LoadData::show_basic_information() {
        std::cout << "show basic information: " << std::endl;
        std::cout << "************************" << std::endl;

        std::cout << "Gpu num: " << GPU_NUM << ",Neuron num: " << NEURON_NUM << ",Pop num: " << POP_NUM << std::endl;
        std::cout << "project root path: " << base_info_ptr->project_root << std::endl;
        //连接概率表、size表和degree表每次不会发生变化，因此这里不再打印
        std::cout << "conn table file path: " << base_info_ptr->conn_dir_path << "/" << base_info_ptr->conn_file_name
                  << std::endl;
        std::cout << "route table file path: " << base_info_ptr->route_dir_path << "/"
                  << base_info_ptr->route_file_name
                  << std::endl;
        std::cout << "map table file path: " << base_info_ptr->map_read_path << "/" << base_info_ptr->map_file_name
                  << std::endl;
        std::cout << "conn dict len: " << conn_dict_table.size() << std::endl;
        std::cout << "************************" << std::endl;
    }


    void LoadData::confirm_conn_table() {
        /*
         * 因为连接概率表几乎不会发生变化，因此可以直接去表里挑选几个数据,来做判断。
         * 同时也要判断连接概率表大小是否对的上
         */

        std::array<double, 5> test_key{2.546736480000000000e+09, 1.070815308000000000e+10, 1.657597232400000000e+10,
                                       2.266836647600000000e+10, 2.941499406300000000e+10};
        std::array<double, 5> test_value{8.483522204173320896e-04, 9.845383854692091492e-04, 9.608414710482950268e-04,
                                         2.497775654454134913e-04, 1.428571428571428492e-01};
        for (gpu_size_type i = 0; i != test_key.size(); ++i) {
            assert_expression("load conn error", conn_dict_table[test_key[i]] == test_value[i]);
        }
        assert_expression("load conn error!", conn_dict_table.size() == 17330810);   // 断言连接概率表长度是否正确
    }

    void LoadData::confirm_map_table() {
        std::array<double, POP_NUM> pop_size_sum{};
        for (auto gpu_map_it = map_table.begin(); gpu_map_it != map_table.end(); ++gpu_map_it) {
            for (auto const &pop_idx_per: *gpu_map_it) {
                pop_size_sum[pop_idx_per.first] += pop_idx_per.second;
            }
        }
        assert_expression("load map table filed!",
                          std::all_of(pop_size_sum.begin(), pop_size_sum.end(), [](double a) { return a == 1; }));
    }

    void LoadData::confirm_load_data() {
        confirm_conn_table();
        confirm_map_table();
        confirm_route_table(default_route_table);
    }

}







