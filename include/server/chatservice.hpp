#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <mutex>
#include "usermodel.hpp"
#include "redis.hpp"
#include "offlinemessageModel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

class ChatService
{
public:
    static ChatService *instance();
    // 处理客户端注册信息
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理客户端注册信息
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    MsgHandler getHandler(int Msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 服务器异常退出
    void reset();

    // 一对一聊天消息处理
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友处理
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //添加群组成员
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);


    //处理Redis的消息函数
    void handlerRedisSubscirbeMessage(int ,string);
private:
    ChatService();
    unordered_map<int, MsgHandler> _msgHandlerMap;
    static ChatService *m_instance;
    // 存储用户连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 线程互斥锁
    mutex _connMutex;
    // 数据操作对象
    UserModel _userModel;
    OfflineMsgModel _offlinemesModel;
    // 存储好友信息
    FriendModel _friendmodel;
    // 调用与群组相关的消息
    GroupModel _groupModel;

    //Redis对象
    Redis _redis;
};

#endif