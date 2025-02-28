#ifndef USER_H
#define USER_H

#include <string>

class User{
public:
    User(int id = -1,std::string name = "",std::string password = "",std::string state = "offline");
    void SetId(int id);
    void SetName(std::string name);
    void SetPassword(std::string password);
    void SetState(std::string state);
    int GetId();
    std::string GetName();
    std::string GetPassword();
    std::string GetState();
private:
    int id_m;
    std::string name_m;
    std::string password_m;
    std::string state_m;

};
#endif