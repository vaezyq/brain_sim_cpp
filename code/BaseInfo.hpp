
#pragma once   //不加这句话当对一个hpp文件重复include后，会出问题

#include <string>
#include <memory>
#include "Utils.hpp"

//bool flag = true;
////#if __cplusplus >= 201703L
//#if flag
//
//#else
//#  include <experimental/filesystem>
//namespace fs = std::experimental::filesystem;
//#endif

# include <filesystem>

namespace fs = std::filesystem;

namespace dtb {
    constexpr unsigned int GPU_NUM = 2000;        //模拟的GPU的个数
    constexpr double NEURON_NUM = 1e10;          //模拟的神经元个数
    constexpr unsigned int POP_NUM = 171508;      //模拟的population的个数


    class BaseInfo {    //基本路径的配置加载类
    public:  //目录路径
        const std::string project_root = fs::current_path().parent_path();       //项目路径为code代码的上一层，这作为文件的工作路径
//        const std::string project_root = "/public/home/ssct005t/project/wml_istbi/brain_sim";       //项目路径
        const std::string tables_path = project_root + "/tables";       //表数据路径
        const std::string route_path = tables_path + "/route_tables";     //路由表路径
        const std::string conn_path = tables_path + "/conn_tables";     //连接概率表路径
        const std::string map_path = tables_path + "/map_tables";     //连接概率表路径
        const std::string traffic_tables_path =
                tables_path + "/traffic_tables/traffic_" + std::to_string(GPU_NUM) + "/";//流量表路径

    public:    //文件路径
        const std::string route_file_name = "route_default_40_50.txt";   //路由表文件名

        const std::string conn_file_name = "conn_dict_int.txt";     //连接概率表文件名

        const std::string map_file_name = "map.txt";        //map表文件名

        const std::string size_file_name = "size.txt";   //size表文件名

        const std::string degree_file_name = "degree.txt";    //degree表文件名

    public:
        /*!
         * 若上述目录不存在则创建，创建上述目录
         * @return
         */
        static std::shared_ptr<BaseInfo> getInstance();

        BaseInfo(const BaseInfo &) = delete;

        BaseInfo &operator=(const BaseInfo &) = delete;

    private:

        static std::shared_ptr<BaseInfo> instance_ptr;     //单例模式中唯一的实例

        BaseInfo() = default;

    };

    std::shared_ptr<BaseInfo> BaseInfo::instance_ptr = nullptr;

    std::shared_ptr<BaseInfo> BaseInfo::getInstance() {
        if (!instance_ptr) {
            //这里不能用make_shared
            // https://www.coonote.com/cplusplus-note/cpp-singleton-mode-summary.html
            instance_ptr = std::shared_ptr<BaseInfo>(new BaseInfo());      //得到实例

//            fs::create_directories(instance_ptr->project_root);
//            fs::create_directories(instance_ptr->tables_path);
            fs::create_directories(instance_ptr->route_path);      //迭代创建文件夹目录
            fs::create_directories(instance_ptr->conn_path);
            fs::create_directories(instance_ptr->traffic_tables_path);
            fs::create_directories(instance_ptr->map_path);

        }
        return instance_ptr;
    }
}







