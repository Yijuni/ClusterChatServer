#ifndef GROUP_H
#define GROUP_H
#include <vector>
#include <string>
#include "GroupUser.h"
class Group{
public:
    Group(int id = -1,std::string name="",std::string desc="");
    void SetId(int id);
    int GetId();
    std::string GetName();
    std::string GetDesc();
    std::vector<GroupUser>& GetUsers();
private:
    int id_m;
    std::string name_m;
    std::string desc_m;
    std::vector<GroupUser> users_m;
};
#endif