#pragma once

#include <unordered_map>
#include <fstream>
#include <memory>
#include <string>
#include "BaseInfo.hpp"
#include <cassert>
#include <array>


/*
 * todo: 加载数据的部分或许可以参考下述链接进行重构
 * 目前存储的为txt文件，因此在读取上依赖于stringstream的构造,当构造了很多stringstream时会降低读取速度
 * 目前读取全部文件耗时大概23s
 * https://stackoverflow.com/questions/71661178/faster-way-of-loading-big-stdvectorstdvectorfloat-from-file
 * https://stackoverflow.com/questions/54238956/fastest-way-to-read-a-vectordouble-from-file
 */

namespace dtb {

    class LoadData {
    private:
        static std::shared_ptr<LoadData> instance_ptr;      //读取数据函数设计为单例,通过instance_ptr管理

        LoadData();

        static std::shared_ptr<BaseInfo> base_info_ptr;     //基本信息类单例指针

        std::unordered_map<double, double> conn_dict_table;   //连接表数据,key-value形式

        std::array<double, POP_NUM> size_table; //体素大小数据

        std::array<gpu_size_type, POP_NUM> degree_table; //体素出度数据

        std::vector<std::unordered_map<gpu_size_type, double> > map_table;   //map表数据

        std::array<std::array<gpu_size_type, GPU_NUM>, GPU_NUM> default_route_table;     //默认路由表数据


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
         * 通过路由表的转发验证默认路由表是否正确，当转发10次还无法抵达则说明路由表错误
         */
        void confirm_route_table();

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

        template<typename T>
        static void load_one_dim_traffic_result(T &one_dim_traffic);

        void show_basic_information();

    };

    void LoadData::load_data() {
        //todo:这里应加上验证：验证路由表是否全部走通，大小是否对。验证map表所有map加和是否完整
        load_map(), load_conn(), load_size(), load_degree(), load_route();
        assert(size_table.size() == POP_NUM);        //判断size是否读取正确
        assert(degree_table.size() == POP_NUM);        //判断degree是否读取正确
        assert(map_table.size() == GPU_NUM);         //判断map表是否读取正确
        confirm_load_data();  //剩余验证，因为验证路由表很耗时，有时可以省略这一步
        show_basic_information();
    }


    std::shared_ptr<LoadData> LoadData::instance_ptr = nullptr;

    std::shared_ptr<BaseInfo>  LoadData::base_info_ptr = BaseInfo::getInstance();

    void LoadData::load_conn() {      //这里使用double，使用更小的精度或许可以减少内存占用
        try {
            //这里在4G的内存上运行会报错，内存不够
            std::ifstream conn_dict_int;
            std::string line;
            conn_dict_int.open(base_info_ptr->conn_dir_path + "/" + base_info_ptr->conn_file_name);
            conn_dict_int.exceptions(std::ifstream::badbit);
            long idx = 0;
            while (getline(conn_dict_int, line)) {
                double k{0.0};
                double v{0.0};
                idx++;
                k = strtod(line.substr(0, 24).c_str(), nullptr);
                v = strtod(line.substr(25, 24).c_str(), nullptr);
                conn_dict_table.insert(std::make_pair(k, v));
                conn_dict_table[k] = v;
                std::cout.precision(19);
//            if (idx % 10000 == 0) {
//                std::cout << "k " << k << std::endl;
//                std::cout << "v " << v << std::endl;
//            }
            }
        } catch (std::ios_base::failure &e) {
            std::cout << "load conn table file failed," << e.what() << std::endl;
            std::terminate();
        }
    }

    void LoadData::load_map() {

        std::string map_table_file_path = base_info_ptr->map_read_path + '/' + base_info_ptr->map_file_name;
        try {
            std::ifstream map_table_txt;
            std::string line;
            gpu_size_type k{0};
            double v{0.0};
            map_table_txt.open(map_table_file_path);
            /*
             * 这里参考的https://stackoverflow.com/questions/9670396/exception-handling-and-opening-a-file
             * 和https://stackoverflow.com/questions/3629321/try-catch-block-for-c-file-io-errors-not-working
             * 注意eofbit是不能加的，因为读取文件到末尾一定会出发这个异常
             */
            map_table_txt.exceptions(std::ifstream::badbit);
            gpu_size_type idx = 0;
            map_table.resize(GPU_NUM);
            while (getline(map_table_txt, line)) {
                std::stringstream word(line);//采用字符流格式将读取的str进行空格分隔，并放入k,v中
                while (word >> k) {
                    word >> v;
                    map_table[idx].insert({k, v});
                }
                idx++;
            }
        } catch (std::ios_base::failure &e) {
            std::cout << map_table_file_path << std::endl;
            std::cout << "load map table file failed," << e.what() << std::endl;
            std::terminate();
        }
    }

    void LoadData::load_size() {
        load_vector_data(size_table, base_info_ptr->conn_dir_path + "/" + base_info_ptr->size_file_name, POP_NUM);
    }

    void LoadData::load_degree() {
        load_vector_data(degree_table, base_info_ptr->conn_dir_path + "/" + base_info_ptr->degree_file_name, POP_NUM);
    }

    void LoadData::load_route() {

        try {
            std::ifstream route_table_file;
            std::string line;
            route_table_file.open(base_info_ptr->route_dir_path + "/" + base_info_ptr->route_file_name);
            route_table_file.exceptions(std::ifstream::badbit);
            if (!route_table_file.is_open()) {
                std::cout << "route file open failed" << std::endl;
                return;
            }
            gpu_size_type idx = 0;
            while (getline(route_table_file, line)) {  //每一行都是空格隔开的数字(含有GPU_NUM行)
                /*
                 * todo: 这里只能读取默认路由表，因为每一行必须含有GPU_NUM个数字
                 * 因此这里路由表名采用的是default_route_table,后续若出现不规整路由表则可以定义route_table
                 */
                std::string item;
                std::stringstream text_stream(line);
                gpu_size_type i = 0;
//                std::cout << line << std::endl;
                while (getline(text_stream, item, ' ')) {
                    default_route_table[idx][i++] = stoi(item);     //这里应该是转成unsigned,但是没有这个函数，转成int在这里也不会出错
                }
                idx++;
            }
            route_table_file.close();
        } catch (std::ios_base::failure &e) {
            std::cout << "load route table file failed," << e.what() << std::endl;
            std::terminate();
        }
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
        std::cout << "Gpu num: " << GPU_NUM << ",Neuron num: " << NEURON_NUM << ",Pop num: " << POP_NUM << std::endl;
        std::cout << "project root path: " << base_info_ptr->project_root << std::endl;
        //连接概率表、size表和degree表每次不会发生变化，因此这里不再打印
//        std::cout << "conn table file path: " << base_info_ptr->conn_path << "/" << base_info_ptr->conn_file_name
//                  << std::endl;
        std::cout << "route table file path: " << base_info_ptr->route_dir_path << "/"
                  << base_info_ptr->route_file_name
                  << std::endl;
        std::cout << "map table file path: " << base_info_ptr->map_read_path << "/" << base_info_ptr->map_file_name
                  << std::endl;
        std::cout << "conn dict len: " << conn_dict_table.size() << std::endl;
        //这里可以加一些map表与route表的验证函数调用
    }

    template<typename T>
    void LoadData::load_one_dim_traffic_result(T &one_dim_traffic) {
        std::ifstream traffic_table_file;
        std::string line;
        std::string traffic_file_name = "traffic_table_base_dcu_out_in_1_dimmap_2000_sequential_cortical_v2.txt";

        traffic_table_file.open(base_info_ptr->traffic_read_write_path + "/" + traffic_file_name);
        //todo: 这里是否可以换成try catch
        if (!traffic_table_file.is_open()) {
            std::cout << "traffic table file open failed" << std::endl;
            return;
        }
        gpu_size_type idx = 0;
//        std::cout << idx << std::endl;
        while (getline(traffic_table_file, line)) {  //每一行都是空格隔开的数字(含有GPU_NUM行)
            std::string item;
            //todo: 不构造stringstream会更快，考虑是否有更快的方法
            std::stringstream text_stream(line);
            gpu_size_type i = 0;
            while (std::getline(text_stream, item, ' ')) {
                one_dim_traffic[idx][i++] = stod(item);
            }
//            std::cout << idx << std::endl;
            idx++;
        }
        traffic_table_file.close();
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
        for (gpu_size_type i = 0; i != 5; ++i) {
            assert(conn_dict_table[test_key[i]] == test_value[i]);
        }
        assert(conn_dict_table.size() == 17330810);     // 断言连接概率表长度是否正确
    }

    void LoadData::confirm_map_table() {
        std::array<double, POP_NUM> pop_size_sum{};
        for (auto gpu_map_it = map_table.begin(); gpu_map_it != map_table.end(); ++gpu_map_it) {
            for (auto const &pop_idx_per: *gpu_map_it) {
                pop_size_sum[pop_idx_per.first] += pop_idx_per.second;
            }
        }
        if (!std::all_of(pop_size_sum.begin(), pop_size_sum.end(), [](double a) { return a == 1; })) {
            std::cout << "confirm map table failed" << std::endl;
        }
    }

    void LoadData::confirm_route_table() {
        std::vector<unsigned> step_length(GPU_NUM * GPU_NUM, 0);
        for (gpu_size_type src = 0; src < GPU_NUM; ++src) {
            for (gpu_size_type dst = 0; dst < GPU_NUM; ++dst) {
                unsigned temp_src = src;
                while ((default_route_table)[temp_src][dst] != temp_src) {
                    temp_src = (default_route_table)[temp_src][dst];
                    step_length[src * GPU_NUM + dst] += 1;
                    if (step_length[src * GPU_NUM + dst] >= 10) {
                        std::cout << "confirm route table failed" << std::endl;
                        std::terminate();
                    }
                }
            }
        }
    }

    void LoadData::confirm_load_data() {
        confirm_conn_table();
        confirm_map_table();
        confirm_route_table();
    }
}







