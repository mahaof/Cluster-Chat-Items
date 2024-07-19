#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"
#include <string>
#include <vector>
class GroupModel
{

public:
    // 创建群组
    bool createGroup(Group &group);
    // 进入群组
    void addGroup(int userid, int groupid, string role);
    // 查询群组成员
    vector<Group> queryGroups(int userid);
    // 根据groupid查询用户id列表，除userid自己，主要用户群聊业务给群组其他成员发消息
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif