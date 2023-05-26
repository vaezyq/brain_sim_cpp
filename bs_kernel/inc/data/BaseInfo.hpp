/*******************************************************************************
 * @file
 * @brief 配置基本信息的文件，配置本系统所有文件路径（所有文件路径统一在这里配置，防止出现冲突的现象）
 * @details
 * @author
 * @date       2022-10-23
 * @version    V1.0
 *******************************************************************************/


#pragma once   //不加这句话当对一个hpp文件重复include后，会出问题

#include <string>
#include <memory>

/*
 * 不同版本gcc兼容，解决参考：
 * https://learn.microsoft.com/en-us/cpp/preprocessor/hash-if-hash-elif-hash-else-and-hash-endif-directives-c-cpp?view=msvc-170
 */

#ifdef __has_include
#  if __has_include(<filesystem>)

#    include <filesystem>

#    define filesystem_name std::filesystem
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
#    define filesystem_name std::experimental::filesystem;
#    define experimental_filesystem
#  else
#    define have_filesystem 0
#  endif
#endif

namespace fs = filesystem_name;


namespace dtb {

    //   gpu卡编号,2000卡为[0,2000)的整数,所有gpu卡编号采用unsigned
    using gpu_size_type = unsigned;

    // 所有pop编号采用unsigned,目前非零体素总量为[0,171508),含零总量为[0,227022),
    using pop_size_type = unsigned;

    //  神经元总数为亿级别,最大为860亿,可采用double表示
    using neuron_size_type = double;

    //  流量统计结果,2000卡100亿神经元时结果级别为10e7,使用unsigned一定会溢出,也可以使用unsigned long long,这里直接使用double
    using traffic_size_type = double;

    constexpr gpu_size_type GPU_NUM = 1000;                 //  模拟的GPU的个数
    constexpr neuron_size_type NEURON_NUM = 1e10;           //  模拟的神经元个数
    constexpr pop_size_type POP_NUM = 171508;               //  模拟的population的个数


    class BaseInfo {    // n  基本路径的配置加载类
    public:   //   目录路径
        const std::string project_root = fs::current_path().parent_path();       //   项目路径为code代码的上一层，作为文件的工作路径
        //      const std::string project_root = "/public/home/ssct005t/project/wml_istbi/brain_sim";       //项目路径
        const std::string tables_dir_path = project_root + "/tables";       // 表数据路径
        const std::string route_dir_path = tables_dir_path + "/route_tables";     //  路由表(route)路径
        const std::string conn_dir_path = tables_dir_path + "/conn_tables";     //  连接概率表(conn)路径


    public:       //   常用的基本文件名(用作写入文件时)
        const std::string iter_map_dir = "iter_size_balanced_map_by_balancing_traffic";
        const std::string gene_sequential_map_name = "sequential_map";
        const std::string gene_size_balanced_name = "size_degree_balanced_map";

    public:   //  * map_table 目录路径

        const std::string map_base_path = tables_dir_path + "/map_tables";

        const std::string map_read_path = map_base_path + "/" + "map_" + std::to_string(GPU_NUM);

        const std::string map_write_path = map_base_path + "/" + "map_" + std::to_string(GPU_NUM) + "/" + iter_map_dir;

    public:   // * traffic table 目录路径

        const std::string traffic_base_path = tables_dir_path + "/traffic_tables";

        const std::string traffic_read_write_path =
                traffic_base_path + "/" + "traffic_" + std::to_string(GPU_NUM);// * 流量表读写路径

    public:    // * 常用表数据文件名
        const std::string route_file_name = "route_phy_1000";   // * 路由表文件名

        const std::string conn_file_name = "conn_dict_int";     // * 连接概率表文件名

        const std::string map_file_name = "map_1000_after_size_balance_without_invalid_index.txt";        // * map表文件名

        const std::string size_file_name = "size";   // * size表文件名

        const std::string degree_file_name = "degree";    // * degree表文件名

        const std::string traffic_file_name = "traffic_table_out_in_2_dim_map_2000_balance_size_map.txt";       // * traffic文件名

    public:


        /*!
         * BaseInfo类设计为singleton,目前在多线程访问Instance之前会已经构造出唯一实例，所以没有采用多线程写法，后续若有问题需添加双检锁
         * @return
         */
        static std::shared_ptr<BaseInfo> getInstance();

        /*!
         * 单例模式不允许复制构造函数
         */
        BaseInfo(const BaseInfo &) = delete;

        /*!
         * 单例模式不允许拷贝构造函数
         * @return
         */
        BaseInfo &operator=(const BaseInfo &) = delete;

    private:

        static std::shared_ptr<BaseInfo> instance_ptr;     // * 单例模式中唯一的实例

        BaseInfo() = default;

    };

    std::shared_ptr<BaseInfo> BaseInfo::instance_ptr = nullptr;

    std::shared_ptr<BaseInfo> BaseInfo::getInstance() {
        if (!instance_ptr) {
            /*
             * 这里不能用make_shared,make_shared虽然更快，但是它需要自己调用BaseInfo的构造函数，
             * 这里把BaseInfo的构造函数设为私有了，无法被make_shared内部调用
             * 单例模式参考：https://www.coonote.com/cplusplus-note/cpp-singleton-mode-summary.html
             * 因为第一次单例被创建后，需要做一些文件夹创建的操作，所以这里没有使用线程安全也最简单的static方式实现，而是使用的智能指针
             */

            // * instance_ptr=std::make_shared<BaseInfo>();   这两个方法都可以构造,make_shared的方法不需要在括号内再new对象,使用构造函数的一定需要

            instance_ptr = std::shared_ptr<BaseInfo>(new BaseInfo());

//            fs::create_directories(instance_ptr->project_root);
//            fs::create_directories(instance_ptr->tables_path);      //  因为是递归创建，所以根目录两行不需要再被创建
            fs::create_directories(instance_ptr->route_dir_path);      //  * 递归创建文件夹目录
            fs::create_directories(instance_ptr->conn_dir_path);
            fs::create_directories(instance_ptr->traffic_read_write_path);
            fs::create_directories(instance_ptr->map_read_path);
            fs::create_directories(instance_ptr->map_write_path);
        }
        return instance_ptr;
    }
}







