#pragma once
#include<iostream>
#include<ctime>
#include<string>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<vector>
#include<unordered_map>
#include<unistd.h>
#include<sys/stat.h>
#include<sstream>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<cstdlib>
#include<sys/wait.h>
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map; 
using std::stringstream;
#define INFO 1
#define WARNING 2
#define ERROR 3
#define FALTA 4
#define LOG(level,message)  Log(#level,message,__FILE__,__LINE__)
void Log(string level,string message,string file_name,int line){
  
  //显示当前事件
  char now[64]={0};
  time_t tt=time(nullptr);
  struct tm*ttime;
  ttime=localtime(&tt);
  strftime(now,64,"%Y-%m-%d %H:%M:%S",ttime);
  cout<<"["<<now<<""<<"["<<level<<"]"<<"["<<message<<"]"<<"["<<file_name<<"]"<<"["<<line<<"]"<<endl;

}

