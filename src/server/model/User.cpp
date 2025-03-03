#include "User.h"

User::User(int id, std::string name, std::string password, std::string state)
{
    id_m = id;
    name_m = name;
    password_m = password;
    state_m = state;
}


void User::SetId(int id)
{
    id_m = id;
}

void User::SetName(std::string name)
{
    name_m = name;
}

void User::SetPassword(std::string password)
{
    password_m = password;
}

void User::SetState(std::string state)
{
    state_m = state;
}

int User::GetId()
{
    return id_m;
}

std::string User::GetName()
{
    return name_m;
}

std::string User::GetPassword()
{
    return password_m;
}

std::string User::GetState()
{
    return state_m;
}
