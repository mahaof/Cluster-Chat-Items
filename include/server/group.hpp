#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"

#include <vector>
#include <string>
using namespace std;
class Group
{
public:
    Group(int i=-1,string name="",string desc=""){
        this->id=id;
        this->groupName=name;
        this->groupDesc=desc;
    }
    void setId(int id) { this->id = id; }
    void setGName(string groupName) { this->groupName = groupName; }
    void setGDesc(string groupDesc) { this->groupDesc = groupDesc; }
    int getId() { return this->id; }
    string getGName() { return this->groupName; }
    string getGDesc() { return this->groupDesc; }
    vector<GroupUser>&getGroupUser(){ return this->users;}
private:
    int id;
    string groupName;
    string groupDesc;
    vector<GroupUser> users;
};
#endif