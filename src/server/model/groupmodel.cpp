#include "groupmodel.hpp"
#include "db.h"
#include <muduo/base/Logging.h>
// 创建群组
bool GroupModel::createGroup(Group &group)
{

    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "insert into AllGroup(groupname,groupdesc)values('%s','%s')", group.getGName().c_str(), group.getGDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO << "群组创建成功";
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
// 进入群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s')", userid, groupid, role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO << "用户加入群组成功";
        }
    }
}
// 查询群组成员
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id=b.groupid where userid=%d", userid);
    vector<Group> gvec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;

                group.setId(atoi(row[0]));
                group.setGName((row[1]));
                group.setGDesc((row[2]));
                gvec.push_back(group);

                LOG_INFO << "群组信息查询成功";
            }
        }
        mysql_free_result(res);
    }
    for (Group &group : gvec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from Users a inner join GroupUser\
                                 b on a.id=b.userid where b.groupid=%d",
                group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser guser;

                guser.setId(atoi(row[0]));
                guser.setName((row[1]));
                guser.setState((row[2]));
                guser.setGroupRole((row[3]));
                group.getGroupUser().push_back(guser);

                LOG_INFO << "群组用户信息查询成功";
            }
        }
        mysql_free_result(res);
    }
    return gvec;
}
// 根据groupid查询用户id列表，除userid自己，主要用户群聊业务给群组其他成员发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "select userid from GroupUser where groupid=%d and userid!=%d", groupid, userid);
    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));

                LOG_INFO << "群组信息查询成功";
            }
        }
        mysql_free_result(res);
    }
    return idVec;
}