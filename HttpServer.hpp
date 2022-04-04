#include"TcpServer.hpp"

#include"ThreadPool.hpp"

#define PORT 8080
#include<signal.h>


class HttpServer{
  private:
    int listen_sock;
    int port;
    TcpServer*svr;

  public:
    HttpServer(int _port=PORT):listen_sock(-1),port(_port){

    }

    //初始化服务器
    void InitHttpServer(){
      svr=TcpServer::getinstance(port);
      svr->InitServer();
      listen_sock=svr->Get_Listen_Sock();
      signal(SIGPIPE,SIG_IGN);
      LOG(INFO,"InitHttpServer Success!");
    }

    //启动服务器
    void Loop(){
     
      struct sockaddr_in peer;
      memset(&peer,0,sizeof(peer));
      socklen_t len=sizeof(peer);
      while(1){
          int cfd=accept(listen_sock,(struct sockaddr*)&peer,&len);
          if(cfd<0){
            LOG(WARNING,"accept error!");
            sleep(2);
            continue;
          }
          LOG(INFO,"Get a New Link!");
          Task task(cfd);
          ThreadPool::getinstance()->Push(task);
          
          LOG(INFO,"Client ----------------------------------------");
          
      }
    }

    
    ~HttpServer(){
      if(listen_sock>=0){
        close(listen_sock);
      }
    }

};


