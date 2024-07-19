#ifndef GROUPUSER_H
#define GROUPUSER_H

#include <string>
#include"user.hpp"
using namespace std;

class GroupUser:public User
{
public:
    void setGroupRole(string grouprole) { this->grouprole = grouprole; }
    string getGroupRole() { return this->grouprole; }

private:
    string grouprole;
};

#endif