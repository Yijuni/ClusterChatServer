#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "User.h"
class GroupUser:public User{
public:
    void SetRole(std::string role);
    std::string GetRole();

private:
    std::string role_m;
};
#endif