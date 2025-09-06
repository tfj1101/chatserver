#include <iostream>
#include "include/json.hpp"
using json = nlohmann::json;
using namespace std;

int main()
{
    json j = {
        {"name", "张三"},
        {"age", 22},
        {"city", "广州"},
        {"group",{
            1,"name1",
            2,"name2"}}};

    // 序列化为字符串
    string s = j.dump(4);
    cout << "序列化后的json：\n"
         << s << endl;

    // 解析json字符串
    string json_string = R"({"name": "李四", "age": 30})";
    json parsed = json::parse(json_string);

    cout << "name = " << parsed["name"] << endl;
    cout << "age = " << parsed["age"] << endl;
    cout<<"group 1 "<<j["group"][0]<<endl;

    return 0;
}