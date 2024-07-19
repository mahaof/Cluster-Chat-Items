#ifndef USERMODEL_H
#define USERMODEL_H
#include"user.hpp"

class UserModel{
public:
    bool insert(User &user);
    //根据用户id查询用户信息
    User query(int id);
    bool updateState(User user);
    void resetState();
};


#endif