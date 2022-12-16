//
// Created by 王明龙 on 2022/12/16.
//

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


class A {
public:
    A(int argc, char **argv) {

    }

};

struct IoFail : public std::ios_base::failure {
    const char *what() const noexcept override //<---**** Stared statement.
    {
        return "My exception happened";
    }
};

int main() {
    int argc;
    char **argv;
    A a(argc, argv);



    std::ifstream file_data;
    std::string line;
//    std::string file_path = "";
//    try {
//        int idx = 0;
//        file_data.open(file_path);
//        file_data.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
//    } catch (const IoFail &e) {
//        std::cout << e.what() << std::endl;
//    }


    try {
        std::ifstream map_table_txt;
        std::string line;
        int k{0};
        double v{0.0};
        map_table_txt.open("/home/wml/brain_sim/tables/map_tables/map.txt");
        file_data.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
        int idx = 0;
//        map_table.resize(GPU_NUM);
        while (getline(map_table_txt, line)) {
            std::stringstream word(line);//采用字符流格式将读取的str进行空格分隔，并放入k,v中
            cout << line;
            idx++;
        }
    } catch (std::ios_base::failure &e) {
        std::cout << "/home/wml/brain_sim/tables/map_tables/map.txt" << std::endl;
        std::cout << "load map table file failed," << e.what() << std::endl;
//        std::terminate();
    }

}