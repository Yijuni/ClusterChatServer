#include "Group.h"

Group::Group(int id, std::string name, std::string desc):
    id_m(id),name_m(name),desc_m(desc)
{
}

void Group::SetId(int id)
{
    id_m=id;
}

int Group::GetId()
{
    return id_m;
}

std::string Group::GetName()
{
    return name_m;
}

std::string Group::GetDesc()
{
    return desc_m;
}

std::vector<GroupUser> &Group::GetUsers()
{
    // TODO: 在此处插入 return 语句
    return users_m;
}
