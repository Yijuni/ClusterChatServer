#include "json.hpp"
using json = nlohmann::json;
#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

//  json序列化
string func(){
    json js;
    js["type"] = 2;
    js["from"] = "moyoj";
    js["to"] = "Marisa";
    js["msg"] = "please don't use master-spark";
    cout<<js<<endl;
    string sendMsg = js.dump();
    cout<<sendMsg<<endl;
    return sendMsg;
}



string func1(){
    json js;
    js["id"] = {11,45,14};
    js["msg1"]["moyoj"] = "baka！";
    js["msg1"]["Marisa"] = "yellow-white";
    //和以上代码同等效果
    js["msg"] = {{"lingmeng","wu nv"},{"da jiang","friend"}};
    cout<<js<<endl;
    return js.dump();
}

string func2(){
    json js;
    vector<int> vec;
    vec.emplace_back(1);
    vec.emplace_back(2);
    vec.emplace_back(3);

    js["list"] = vec;

    map<int,string> m;
    m.insert({9,"琪露诺"});
    m.insert({10,"觉"});
    js["name"] = m;
    return js.dump();
}

int main(){
    cout<<"<<<<<<<<<<<<<func"<<endl;
    string recvMsg = func();
    //数据反序列化
    json parse = json::parse(recvMsg);
    cout<<parse["from"]<<" "<<parse["msg"]<<endl;
    cout<<"<<<<<<<<<<<<<<<func1"<<endl;
    recvMsg = func1();
    parse = json::parse(recvMsg);
    cout<<parse["id"]<<endl;
    json msgjs = parse["msg"];
    cout<<msgjs["lingmeng"]<<endl;
    cout<<"<<<<<<<<<<<<<func2"<<endl;
    recvMsg = func2();
    cout<<recvMsg<<endl;
    parse = json::parse(recvMsg);
    vector<int> vec = parse["list"];
    map<int,string> m = parse["name"];
    for(int i=0;i<vec.size();i++){
        cout<<vec[i]<<endl;
    }
    cout<<m[9]<<endl;

    return 0;
}