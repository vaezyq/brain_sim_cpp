//
// Created by 王明龙 on 2022/12/16.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "data/LoadData.hpp"
#include "document.h"
#include "istreamwrapper.h"
using namespace std;


class A {
public:
    virtual void func() {}

};

class C {
public:

};

class B : public A, public C {
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



using namespace rapidjson;


void AddBasicType(Document &d, Document::AllocatorType &allocator) {

    d.AddMember("height", 170.5, allocator);      // 添加浮点类型
    d.AddMember("subject1", "math", allocator);   // 添加字符串类型,常量方式

    string strSubject = "English";
    Value valueSubject(kStringType);
    valueSubject.SetString(strSubject.c_str(), strSubject.size(), allocator);

    if (!valueSubject.IsNull()) {
        d.AddMember("subject2", valueSubject, allocator); // 添加字符串类型，变量方式，不能直接对变量进行添加，类型不对
    }
}

void ParseBasicType(Document &d, string jsonData) {
    if (!d.Parse(jsonData.data()).HasParseError()) {
        // 解析整型
        if (d.HasMember("digit") && d["digit"].IsInt()) {
            cout << "digit = " << d["digit"].GetInt() << endl;
        }

        // 解析浮点型
        if (d.HasMember("height") && d["height"].IsDouble()) {
            cout << "height = " << d["height"].GetDouble() << endl;
        }

        // 解析字符串类型
        if (d.HasMember("subject1") && d["subject1"].IsString()) {
            cout << "subject1 = " << d["subject1"].GetString() << endl;
        }

        if (d.HasMember("subject2") && d["subject1"].IsString()) {
            cout << "subject2 = " << d["subject2"].GetString() << endl;
        }
    }
}

int main() {


    string path = dtb::BaseInfo::getInstance()->route_dir_path + "/conn.json";

    std::ifstream file(path);
    std::string strJson((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse(strJson.c_str());

    if (doc.HasParseError())
    {
        printf("解析json文件失败，请检查json是否正确!\n");
    }


//    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
//    Document d;
//    d.Parse(path.c_str());


//    double x = 1.000000000000000000e+00;
//    for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
//        printf("Type of member %s is %f\n", itr->name.GetString(), itr->value.GetDouble());
//    }

    if (doc.HasMember("29414994062.000000")) {
        cout << doc["29414994062.000000"].GetDouble();
    }


//    auto load_data_ptr = dtb::LoadData::getLoadDataInstance();
//
//    vector<string> vec(load_data_ptr->getConnDictTable().size(), "");
//
//
//    Document d;
//    d.SetObject();
//    wstring wsValue;
//    Document::AllocatorType &allocator = d.GetAllocator(); // 获取分配器
//
//
//    int count = 0;
//    for (auto iter = load_data_ptr->getConnDictTable().begin();
//         iter != load_data_ptr->getConnDictTable().end(); ++iter) {
//
//        vec[count] = to_string(iter->first);
//
//        d.AddMember(StringRef(vec[count].c_str()), iter->second, allocator);           // 添加整型数据
//        count++;
//
//        if (count % 1000 == 0) {
//            cout << count << endl;
//        }
//    }
//
//
//    rapidjson::StringBuffer buf;
//
//    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
//
//    d.Accept(writer);
//
//    string path = dtb::BaseInfo::getInstance()->route_dir_path + "/conn.json";
//
//    const char *file_name = path.c_str();
//
//    std::ofstream outfile;
//    outfile.open(file_name);
//    if (!outfile.is_open()) {
//        fprintf(stderr, "fail to open file to write: %s\n", file_name);
//        return -1;
//    }
//
//    outfile << buf.GetString() << std::endl;


//    for (Value::ConstMemberIterator itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr) {
//        printf("Type of member %s is %f\n", itr->name.GetString(), itr->value.GetDouble());
//    }




//    if (d.HasMember("digit")) {
//        cout << d["digit"].GetDouble();
//    }


//    StringBuffer strBuf;
//    Writer<StringBuffer> writer(strBuf);
//    d.Accept(writer);
//
//    string jsonData = strBuf.GetString();
//    cout << jsonData << endl << endl;
//
//    ParseBasicType(d, jsonData);        // 解析基本类型
//    nlohmann::json j;
//    j[1] = {1, 0, 2};
//    cout << j[1][2];


//    array<int, 5> vec{1, 2, 3, 4, 5};
//    std::sort(vec.begin(), vec.end(), [](auto &a, auto &b) {
//        return a > b;
//    });
//    for_each(vec.begin(), vec.end(), [](int i) { cout << i << " "; });

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