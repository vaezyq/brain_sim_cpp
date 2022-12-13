#include <unordered_map>
#include <fstream>
#include <memory>
#include <string>
#include "Utils.hpp"
#include "BaseInfo.hpp"
#include <cassert>
#include <array>

namespace dtb {


    class LoadData {
    private:


        static std::shared_ptr<LoadData> instance_ptr;      //读取数据函数设计为单例,通过instance_ptr管理

        LoadData();

        static std::shared_ptr<BaseInfo> base_info_ptr;     //基本信息类单例指针

        std::unordered_map<double, double> conn_dict_table;   //连接表数据,key-value形式


        std::array<double, POP_NUM> size_table; //体素大小数据

        std::array<int, POP_NUM> degree_table; //体素出度数据

        std::vector<std::unordered_map<int, double> > map_table;   //map表数据

        std::array<std::array<unsigned, GPU_NUM>, GPU_NUM> default_route_table;     //默认路由表数据


    public:
        void load_data();

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
        //todo: 读取文件时是否存在精度损失
        void confirm_conn_table();

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

        inline const std::array<int, POP_NUM> &getDegreeTable() const;

        inline std::vector<std::unordered_map<int, double>> &getMapTable();

        inline const std::array<std::array<unsigned int, GPU_NUM>, GPU_NUM> &getRouteTable() const;

        LoadData(const LoadData &) = delete;

        LoadData &operator=(const LoadData &) = delete;

    };

    void LoadData::load_data() {
        load_map(), load_conn(), load_size(), load_degree(), load_route();
        assert(size_table.size() == POP_NUM);        //判断size是否读取正确
        assert(degree_table.size() == POP_NUM);        //判断degree是否读取正确
        assert(map_table.size() == GPU_NUM);         //判断map表是否读取正确
        std::cout << "data load finished" << std::endl;
    }


    std::shared_ptr<LoadData> LoadData::instance_ptr = nullptr;

    std::shared_ptr<BaseInfo>  LoadData::base_info_ptr = BaseInfo::getInstance();

    void LoadData::load_conn() {      //这里使用double，使用更小的精度或许可以减少内存占用
        //这里在4G的内存上运行会报错，内存不够
        std::ifstream conn_dict_int;
        std::string line;
        double k{0.0};
        double v{0.0};
        conn_dict_int.open(base_info_ptr->conn_path + "/" + "conn_dict_int.txt");
        if (!conn_dict_int.is_open()) {
            std::cout << "conn file open failed" << std::endl;
            return;
        }
        long idx = 0;
        while (getline(conn_dict_int, line)) {

//            string s{"1.186222063600000000e+10 6.105891744189615106e-04"};
//            cout << s.substr(0,24) << endl;
//            double k = strtod(s.substr(0,24).c_str(), nullptr);
//            cout.precision(19);
//
//            std::stringstream word(line);//采用字符流格式将读取的str进行空格分隔，并放入k,v中
//            word.precision(20);
//            word >> k;
//            word >> v;

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
    }

    void LoadData::confirm_conn_table() {
        std::ifstream conn_dict_int;
        std::string line;
        double k{0.0};
        double v{0.0};
        conn_dict_int.open(base_info_ptr->conn_path + "/" + "conn_dict_int.txt");
        if (!conn_dict_int.is_open()) {
            std::cout << "conn file open failed" << std::endl;
            return;
        }
        long idx = 0;
        while (getline(conn_dict_int, line)) {
            idx++;
            k = strtod(line.substr(0, 24).c_str(), nullptr);
            v = strtod(line.substr(25, line.size()).c_str(), nullptr);

            if (conn_dict_table[k] != v) {
                std::cout << "failed" << std::endl;
            }
        }


    }

    void LoadData::load_map() {

        std::ifstream map_table_txt;
        std::string line;
        int k{0};
        double v{0.0};
        map_table_txt.open(base_info_ptr->conn_path + "/" + "map.txt");

        if (!map_table_txt.is_open()) {
            std::cout << "conn file open failed" << std::endl;
            return;
        }
        int idx = 0;
        map_table.resize(GPU_NUM);
        while (getline(map_table_txt, line)) {

            std::stringstream word(line);//采用字符流格式将读取的str进行空格分隔，并放入k,v中
            while (word >> k) {
                word >> v;
                map_table[idx].insert({k, v});
//            if (idx == 13) {
//                std::cout << k << " " << v << " " << std::endl;
//            }
            }
            idx++;
        }
    }

    void LoadData::load_size() {
        load_vector_data(size_table, base_info_ptr->conn_path + "/" + "size.txt", 171508);
    }

    void LoadData::load_degree() {
        load_vector_data(degree_table, base_info_ptr->conn_path + "/" + "degree.txt", 171508);
    }

    const std::array<double, POP_NUM> &LoadData::getSizeTable() const {
        return size_table;
    }

    const std::array<int, POP_NUM> &LoadData::getDegreeTable() const {
        return degree_table;
    }


    const std::unordered_map<double, double> &LoadData::getConnDictTable() const {
        return conn_dict_table;
    }


    std::vector<std::unordered_map<int, double>> &LoadData::getMapTable() {
        return map_table;
    }

    const std::array<std::array<unsigned int, GPU_NUM>, GPU_NUM> &LoadData::getRouteTable() const {
        return default_route_table;
    }

    std::shared_ptr<LoadData> LoadData::getLoadDataInstance() {
        if (!instance_ptr) {   //若还没有初始化，即第一次加载数据
            instance_ptr = std::shared_ptr<LoadData>(new LoadData());   //构造对象并在构造函数内加载数据
        }
        return instance_ptr;
    }

    void LoadData::load_route() {
        std::ifstream route_table_file;
        std::string line;
        route_table_file.open(base_info_ptr->route_path + "/" + base_info_ptr->route_file_name);
        //todo: 这里是否可以换成try catch
        if (!route_table_file.is_open()) {
            std::cout << "route file open failed" << std::endl;
            return;
        }
        int idx = 0;
        while (getline(route_table_file, line)) {  //每一行都是空格隔开的数字(含有GPU_NUM行)
            //todo: 这里只能读取默认路由表，因为每一行必须含有GPU_NUM个数字
            for (unsigned i = 0; i < GPU_NUM; ++i) {
                std::stringstream ss(line);//将字符串line放入到输入输出流ss中
                while (ss >> default_route_table[idx][i]);
            }
            idx++;
        }
        route_table_file.close();
    }

    LoadData::LoadData() {
        load_data();
    }
}







