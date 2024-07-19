#include <iostream>
#include"chatserver.hpp"
#include"chatservice.hpp"
#include<signal.h>
using namespace std;

void resHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    // if (agrc < 3)
    // {
    //     cerr << "command invalid! example:./Chatserver 127.0.0.1 6000" << endl;
    // }

    // char *ip = argv[1];
    // uint16_t port = atoi(argv[2]);

    // signal(SIGINT,resHandler);
    // EventLoop loop;
    // InetAddress addr("127.0.0.1",6000);
    // ChatServer _server(&loop,addr,"test");

    // _server.start();
    // loop.loop();
    // return 0;
    if (argc < 3) {
        std::cerr << "command invalid! example: ./Chatserver 127.0.0.1 6000" << std::endl;
        return -1; // 退出程序
    }

    char *ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));

    signal(SIGINT, resHandler);
    EventLoop loop;
    InetAddress addr(ip, port); // 使用动态传入的 IP 和端口
    ChatServer _server(&loop, addr, "test");

    _server.start();
    loop.loop();
    return 0;
}