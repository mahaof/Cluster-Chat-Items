#include "friendmodel.hpp"
#include "db.h"
#include <muduo/base/Logging.h>
#include <vector>

void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "insert into Friend(userid,friendid)values(%d,%d)", userid, friendid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
        LOG_INFO << "好友信息插入成功";
    }
}
vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "SELECT a.id, a.name, a.state FROM Users a INNER JOIN Friend b ON b.friendid = a.id WHERE b.userid = %d", userid);
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);

        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName((row[1]));
                user.setState((row[2]));

                vec.push_back(user);
                LOG_INFO << "好友信息插入成功";
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}