#include "chatservice.hpp"
#include <iostream>

#include <muduo/base/Logging.h>
#include <vector>
#include "group.hpp"
using namespace std;

#include "public.hpp"

// singleton设计单例模式
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 构造函数绑定业务
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    // 创建群组的消息
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});

    // 添加群组组员的消息
    _msgHandlerMap.insert({ADD_USERGROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    // 群组聊天
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscirbeMessage, this, _1, _2));
    }
}

//从redis消息队列中发布消息
void ChatService::handlerRedisSubscirbeMessage(int userid, string message)
{
    lock_guard<mutex> lock(_connMutex);
    auto it=_userConnMap.find(userid);
    if(it!=_userConnMap.end())
    {
        it->second->send(message);
        return ;
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    LOG_INFO << "调用了handler";
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << "can't find handler";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 服务器异常退出
void ChatService::reset()
{
    // 把所有online的用户设置成offline
    _userModel.resetState();
}

// 业务处理---登录
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{

    int id = js["id"];
    string pwd = js["password"];
    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 用户已经登录，不允许重复登录
            json respones;
            respones["msgid"] = LOGIN_MSG_ACK;
            respones["erron"] = 2;
            respones["erroMsg"] = "This account is online,please login another account!!!";
            conn->send(respones.dump());
        }
        else
        {
            LOG_INFO << "login successful!";

            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // 向消息队列订阅id
            _redis.subscribe(id);

            // 登录成功，更新用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            json respones;
            respones["msgid"] = LOGIN_MSG_ACK;
            respones["erron"] = 0;
            respones["sucMsg"] = "Login success!!!";
            respones["id"] = user.getId();
            respones["name"] = user.getName();
            respones["state"] = user.getState();

            // 查询用户是否有离线消息
            vector<string> vec = _offlinemesModel.query(id);
            if (!vec.empty())
            {
                respones["offlinemessage"] = vec;
                // 删除离线消息
                _offlinemesModel.remove(id);
            }

            // 返回用户的好友信息
            vector<User> uservec = _friendmodel.query(id);
            if (!uservec.empty())
            {
                vector<string> vec2;
                for (User user : uservec)
                {
                    LOG_INFO << "具有好友信息";
                    json js;
                    js["friendid"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                respones["friends"] = vec2;
            }

            // 返回群组的信息
            vector<Group> gVec = _groupModel.queryGroups(id);
            if (!gVec.empty())
            {
                vector<string> vec3;
                for (Group group : gVec)
                {
                    json js;
                    js["groupid"] = group.getId();
                    js["groupname"] = group.getGName();
                    js["groupdesc"] = group.getGDesc();
                    vector<string> vec4;
                    for (GroupUser &guser : group.getGroupUser())
                    {
                        json guJs;
                        guJs["id"] = guser.getId();
                        guJs["name"] = guser.getName();
                        guJs["state"] = guser.getState();
                        guJs["role"] = guser.getGroupRole();
                        vec4.push_back(guJs.dump());
                    }
                    js["users"] = vec4;
                    vec3.push_back(js.dump());
                }
                respones["groups"] = vec3;
            }

            // 返回群成员信息

            conn->send(respones.dump());
        }
    }
    else
    {
        // login failed
        json respones;
        respones["msgid"] = LOGIN_MSG_ACK;
        respones["erron"] = 1;
        respones["erroMsg"] = "用户名或者密码错误";
        conn->send(respones.dump());
    }
}

// 业务处理---注册
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_ERROR << "register successful!";

    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        json respones;
        respones["msgid"] = REG_MSG_ACK;
        respones["erron"] = 0;
        respones["id"] = user.getId();
        conn->send(respones.dump());
    }
    else
    {
        json respones;
        respones["msgid"] = REG_MSG_ACK;
        respones["erron"] = 1;
        conn->send(respones.dump());
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    _redis.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{

    int id = js["toid"].get<int>();
    {
        lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 在线转发消息
            it->second->send(js.dump());
            return;
        }
    }
    // 查询用户是否在线
    User user = _userModel.query(id);
    if (user.getState() == "online")
    {
        _redis.publish(id, js.dump());
    }
    _offlinemesModel.insert(id, js.dump());
    // 处理用户离线消息
}

// 添加好友信息
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendmodel.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];

    // 存储新创建的群组消息
    Group group(-1, groupname, groupdesc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 进入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"];
    int groupid = js["groupid"];
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"];
    int groupid = js["groupid"];
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            // 查询用户是否在线
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                _offlinemesModel.insert(id, js.dump());
            }
        }
    }
}
