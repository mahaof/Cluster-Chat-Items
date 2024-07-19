#include "json.hpp"
#include "iostream"
#include <thread>
#include <string>
#include <chrono>
#include <ctime>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登录用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentGroupList;
// 显示当前登录用户的基本信息
void showCurrentData();

// 接受线程
void readTaskHandler(int clientfd);
// 获取系统时间
string getCurrentTime();
// 聊天主页面
void mainMenu(int clientfd);

//
void help(int fd = 0, string str = "");
void chat(int, string);
void addfriend(int, string);
void addgroup(int, string);
void creatorgroup(int, string);
void groupchat(int, string);
void loginout(int, string);

// 系统支持的哭护短命令列表

unordered_map<string, string> commandMap = {
    {"help", "显示所有的支持命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creatorgroup", "创建群组格式creatorgroup:groupname:groupdesc"},
    {"addgroup", "添加群组,格式addgroup:groupid"},
    {"groupchat", "群组聊天,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creatorgroup", creatorgroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 客户端实现，main线程作为发送线程，子线程用作接受线程
int main(int agrc, char **argv)
{
    if (agrc < 3)
    {
        cerr << "command invalid! example:./Chatclient 127.0.0.1 6000" << endl;
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socekt create error" << endl;
        exit(-1);
    }

    // 绑定ip地址
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // main线程用来接收用户的输入，负责发送数据
    for (;;)
    {
        cout << "=============================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        int choice = 0;
        cin >> choice;
        cin.get(); // 去除缓冲区回车

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            // cin.get(); // 读掉缓冲区残留的回车
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send msg error!" << endl;
            }
            else
            {
                char buffer[2048] = {0};
                len = recv(clientfd, buffer, 2048, 0);
                if (-1 == len)
                {
                    cerr << "recv mesage is error" << endl;
                }
                else
                {
                    json responesJs = json::parse(buffer);
                    if (1 == responesJs["erron"].get<int>())
                    {
                        cerr << "id or password is error!!" << endl;
                    }
                    else if (2 == responesJs["erron"].get<int>())
                    {
                        cout << "This user have logged,please login other account!!!" << endl;
                    }
                    if (0 == responesJs["erron"].get<int>())
                    {
                        g_currentUser.setId(responesJs["id"].get<int>());
                        g_currentUser.setName(responesJs["name"].get<string>());
                        g_currentUser.setState(responesJs["state"].get<string>());

                        // if (responesJs.contains("friends"))
                        // {
                        //     vector<string> freVec = responesJs["friends"];
                        //     for (string &fre : freVec)
                        //     {
                        //         json js = json::parse(fre);
                        //         User user;
                        //         user.setId(js["id"].get<int>());
                        //         user.setName(js["name"]);
                        //         user.setState(js["state"]);
                        //         g_currentUserFriendList.push_back(user);
                        //     }
                        // }

                        if (responesJs.contains("friends"))
                        {
                            // cout << "入friend" << endl;
                            // cout << "输出真正的friends"<<responesJs["friends"] << endl;
                            // vector<string> freVec = responesJs["friends"].get<std::vector<std::string>>();

                            // cout << "friend序列化" << endl;
                            // for (auto &fre : freVec)
                            // {
                            //     json js = json::parse(fre);
                            //     User user;
                            //     user.setId(js["id"].get<int>());
                            //     user.setName(js["name"]);
                            //     user.setState(js["state"]);
                            //     g_currentUserFriendList.push_back(user);
                            // }
                            cout << "入friend" << endl;
                            vector<string> freVec = responesJs["friends"].get<std::vector<string>>();
                            for (auto &fre : freVec)
                            {
                                json js = json::parse(fre);
                                User user;
                                user.setId(js["friendid"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }
                        if (responesJs.contains("groups"))
                        {
                            vector<string> groVec = responesJs["groups"].get<std::vector<std::string>>();
                            for (auto &gro : groVec)
                            {
                                json js = json::parse(gro);
                                Group group;
                                group.setId(js["groupid"].get<int>());
                                group.setGName(js["groupname"]);
                                group.setGDesc(js["groupdesc"]);
                                vector<string> gUserVec = js["users"].get<std::vector<std::string>>();
                                for (string &gustr : gUserVec)
                                {
                                    json guJs = json::parse(gustr);
                                    GroupUser guser;
                                    guser.setId(guJs["id"].get<int>());
                                    guser.setName(guJs["name"].get<string>());
                                    guser.setState(guJs["state"].get<string>());
                                    guser.setGroupRole(guJs["role"].get<string>());
                                    group.getGroupUser().push_back(guser);
                                }
                                g_currentGroupList.push_back(group);
                            }
                        }
                        showCurrentData();
                        cout << responesJs["offlinemessage"] << endl;
                        if (responesJs.contains("offlinemessage") && !responesJs["offlinemessage"].is_null())
                        {
                            cout << "-------------------offlinemessage list-----------------------" << endl;
                            if (responesJs.find("offlinemessage") != responesJs.end())
                            {
                                cout << "-------------------offlinemessage list-----------------------" << endl;
                                vector<string> msgVec = responesJs["offlinemessage"];
                                cout << "-------------------offlinemessage list-----------------------" << endl;
                                for (auto &msg : msgVec)
                                {
                                    cout << "-------------------offlinemessage list-----------------------" << endl;
                                    if (!msg.empty()) // 检查msg是否为空
                                    {
                                        json js = json::parse(msg);
                                        cout << "-------------------offlinemessage list-----------------------" << endl;
                                        if (ONE_CHAT_MSG == js["msgid"].get<int>())
                                        {
                                            // cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
                                            //      << "said" << js["msg"].get<string>() << endl;
                                            cout << js["time"] << "[" << js["id"].get<int>() << "]" << js["name"]
                                                 << "said:" << js["message"] << "。" << endl;
                                        }
                                        else
                                        {
                                            // cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
                                            //      << "said" << js["msg"].get<string>() << endl;
                                            cout << js["time"]<<"   " << "[" << js["groupid"].get<int>() << "]"<<"----["<< js["userid"].get<int>() << "]:" << js["username"]
                                                 << "said:" << js["message"] << "。" << endl;
                                        }
                                    }
                                }
                            }
                        }

                        std::thread readTask(readTaskHandler, clientfd);
                        readTask.detach();

                        mainMenu(clientfd);
                    }
                }
            }
        }

        break;
        case 2: // 注册业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send msg error!" << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (-1 == len)
                {
                    cerr << "recv mesage is error" << endl;
                }
                else
                {
                    json responesJs = json::parse(buffer);
                    if (0 != responesJs["erron"].get<int>())
                    {
                        cerr << "注册失败" << endl;
                    }
                    else
                    {
                        cout << "register is successful,userid is" << responesJs["id"]
                             << ",do not forget it" << endl;
                    }
                }
            }
        }
        break;
        case 3: // 退出业务
            close(clientfd);
            exit(0);
            break;

        default:
            break;
        }
    }
}

// 显示当前登录用户的基本信息
void showCurrentData()
{
    cout << "=========================================================" << endl;
    cout << "Current login: user=>id:" << g_currentUser.getId() << "--------" << "user=>name:" << g_currentUser.getName() << endl;
    cout << "-----------------------friend list------------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << "--------" << user.getName() << "--------" << user.getState() << endl;
        }
    }
    cout << "-----------------------group list-------------------------" << endl;
    if (!g_currentGroupList.empty())
    {
        for (Group &group : g_currentGroupList)
        {
            cout << group.getId() << "--------" << group.getGName() << "--------" << group.getGDesc() << endl;
            for (GroupUser &guser : group.getGroupUser())
            {
                cout << guser.getId() << "----" << guser.getName() << "----" << guser.getState() << "----" << guser.getGroupRole() << endl;
            }
        }
    }
    cout << "=========================================================" << endl;
}

// 接收线程
void readTaskHandler(int clientfd)
{
    cout << "读取消息功能" << endl;
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }
        // 接收ChatServer转发的数据，反序列化
        json js = json::parse(buffer);
        cout << "网络层传输的内容：" << endl;
        cout << buffer << endl;

        if (ONE_CHAT_MSG == js["msgid"].get<int>())
        {
            // cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
            //      << "said" << js["msg"].get<string>() << endl;
            cout << js["time"] << "[" << js["id"].get<int>() << "]" << js["name"]
                 << "said:" << js["message"] << "。" << endl;
        }
        else
        {
            // cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
            //      << "said" << js["msg"].get<string>() << endl;
            cout << js["time"] << "[" << js["groupid"].get<int>() << "]" << "-----[" << js["userid"]<< "]" <<js["name"]
                 << "said:" << js["message"] << "。" << endl;
        }
    }
}

void mainMenu(int clientfd)
{
    help();
    char buffer[1024] = {0};
    for (;;)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command" << endl;
            continue;
        }
        // 调用相应的命令事件处理回调，mainMenu对修改封闭，添加功能不需要修改改函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

void help(int, string)
{
    if (!commandMap.empty())
    {
        for (const auto it : commandMap)
        {
            cout << it.first << ":        " << it.second << endl;
        }
    }
}
void addfriend(int clientfd, string str)
{
    cout << "进入添加好友功能" << endl;
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();
    cout << "添加好友信息：" << buffer << endl;
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error!!!" << endl;
    }
}
void addgroup(int clientfd, string str)
{
    cout << "进入群组功能" << endl;
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_USERGROUP_MSG;
    js["userid"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error!!!" << endl;
    }
}

void chat(int clientfd, string str)
{
    int idx = str.find(":");
    int friendid = atoi(str.substr(0, idx).c_str());
    string msg = str.substr(idx + 1, (str.size() - idx));
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["message"] = msg;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error!!!" << endl;
    }
}

void creatorgroup(int clientfd, string str)
{
    int idx = str.find(":");
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, (str.size() - idx));
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error!!!" << endl;
    }
}
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    int groupid = atoi(str.substr(0, idx).c_str());
    string msg = str.substr(idx + 1, (str.size() - idx));
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["userid"] = g_currentUser.getId();
    js["username"]=g_currentUser.getName();
    js["groupid"] = groupid;
    js["message"] = msg;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error!!!" << endl;
    }
}
void loginout(int clientfd, string str = "")
{
    close(clientfd);
    exit(-1);
}

// 返回当前时间
string getCurrentTime()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为 time_t 类型
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // 转换为 tm 结构体
    std::tm now_tm = *std::localtime(&now_time_t);

    // 使用 stringstream 格式化时间
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    // 返回格式化后的时间字符串
    return ss.str();
}
