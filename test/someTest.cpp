//
// Created by 王明龙 on 2022/12/16.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>


using namespace std;


class A {
public:
    virtual void func() {}

};

class C {
public:

};

class B : public A ,public C{
public:
    void func() override {
        A::func();
    }
};


struct IoFail : public std::ios_base::failure {
    const char *what() const noexcept override //<---**** Stared statement.
    {
        return "My exception happened";
    }
};

bool is_in_changed_gpu_lists(std::vector<unsigned int> const &gpu_idxs) {
    std::vector<unsigned> vec{1, 2, 3, 4, 5, 6};
    for (auto it = gpu_idxs.begin(); it != gpu_idxs.end(); ++it) {
        if (std::find(vec.begin(), vec.end(), *it) !=
            vec.end()) {
            return true;
        }
    }
    return false;
}

int main() {


    array<int, 5> vec{1, 2, 3, 4, 5};
    std::sort(vec.begin(), vec.end(), [](auto &a, auto &b) {
        return a > b;
    });
    for_each(vec.begin(), vec.end(), [](int i) { cout << i << " "; });

//    auto sort_indices_ptr = dtb::argsort(vec);
//    for_each(sort_indices_ptr->begin(), sort_indices_ptr->end(), [](int i) { cout << i << " "; });
//    cout << endl;
//    std::cout << is_in_changed_gpu_lists({100});
//    std::string file_path = "";
//    try {
//        int idx = 0;
//        file_data.open(file_path);
//        file_data.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
//    } catch (const IoFail &e) {
//        std::cout << e.what() << std::endl;
//    }

//
//    try {
//        std::ifstream map_table_txt;
//        std::string line;
//        int k{0};
//        double v{0.0};
//        map_table_txt.open("/home/wml/brain_sim/tables/map_tables/map_2000_split_iter_96.txt");
//        file_data.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
//        int idx = 0;
////        map_table.resize(GPU_NUM);
//        while (getline(map_table_txt, line)) {
//            std::stringstream word(line);//采用字符流格式将读取的str进行空格分隔，并放入k,v中
//            cout << line;
//            idx++;
//        }
//    } catch (std::ios_base::failure &e) {
//        std::cout << "/home/wml/brain_sim/tables/map_tables/map_2000_split_iter_96.txt" << std::endl;
//        std::cout << "load map table file failed," << e.what() << std::endl;
////        std::terminate();
//    }

}