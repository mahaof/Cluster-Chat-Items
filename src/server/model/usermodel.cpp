#include "usermodel.hpp"
#include "db.h"
#include <muduo/base/Logging.h>

bool UserModel::insert(User &user)
{
    LOG_INFO<<"用户信息插入成功";
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "insert into Users(name,password,state)values('%s','%s','%s')", user.getName().c_str(),
            user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "select * from Users where id=%d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName((row[1]));
                user.setPwd((row[2]));
                user.setState((row[3]));
                mysql_free_result(res);
                LOG_INFO<<"用户信息查询成功";
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updateState(User user)
{

    char sql[1024] = {0};
    // sprintf赋值语句
    sprintf(sql, "update Users set state='%s' where id=%d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            LOG_INFO<<"用户状态更新成功";
            return true;
        }
    }
    return false;
}

// 重置用户信息
void UserModel::resetState()
{
    char sql[1024] = "update Users set state='offline' where state='online'";
    MySQL mysql;
    if (mysql.connect())
    {
        LOG_INFO<<"重置用户状态";
        mysql.update(sql);
    }
}