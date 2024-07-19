#ifndef PUBLIC_H
#define PUBLIC_H


enum EnMsgType
{
    LOGIN_MSG=1,//登录业务
    LOGIN_MSG_ACK,//登录相应
    REG_MSG,//注册业务
    REG_MSG_ACK,//注册确认业务
    ONE_CHAT_MSG,//一对一聊天消息
    ADD_FRIEND_MSG,//添加好友消息
    CREATE_GROUP_MSG,//创建群组的消息
    ADD_USERGROUP_MSG,//进入群组的消息
    GROUP_CHAT_MSG,//群组聊天

};

#endif