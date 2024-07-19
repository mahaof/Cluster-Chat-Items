#ifndef OFFLINEMESSAGE_H
#define OFFLINEMESSAGE_H

#include<iostream>
#include<vector>
#include<string>
using namespace std;

class OfflineMsgModel{

public:
    void insert(int userid, string msg);
    void remove(int userid);
    vector<string> query(int userid); 

};


#endif