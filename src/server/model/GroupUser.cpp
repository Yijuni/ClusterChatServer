#include "GroupUser.h"

void GroupUser::SetRole(std::string role)
{
    role_m = role;
}

std::string GroupUser::GetRole()
{
    return role_m;
}
