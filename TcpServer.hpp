#include"Log.hpp"
class TcpServer{
  private:
    int listen_sock;
    int port;
    static TcpServer*svr;
  public:

    //使用单例模式
    static TcpServer*getinstance(int port){
      static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
      if(svr==nullptr)
      {
        pthread_mutex_lock(&lock);
        if(svr==nullptr)
        {
          svr=new TcpServer(port);
        }
        pthread_mutex_unlock(&lock);
      }
      return svr;
    }
    
    //构造函数
    TcpServer(int _port):listen_sock(-1),port(_port){
      
    }
    
    //析构函数
    ~TcpServer(){
      if(listen_sock>=0){
        close(listen_sock);
      }
    }

    //初始化服务器
    void InitServer(){

      Create_Sock();
      Bind_Sock();
      Listen_Sock();
    }

    //创建套接字
    void Create_Sock(){
      listen_sock=socket(AF_INET,SOCK_STREAM,0);
      if(listen_sock<0){
        LOG(FALTA,"socket error!");
        exit(1);
      }
      int flag=1;
      setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
      LOG(INFO,"socket success!");
    }

    //bind
    void  Bind_Sock(){
      struct sockaddr_in local;
      local.sin_family=AF_INET;
      local.sin_port=htons(port);
      local.sin_addr.s_addr=INADDR_ANY;

      if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local))<0){
        LOG(FALTA,"bind error!");
        exit(1);
      }
      LOG(INFO,"bind success!");
    }

    //listen
    void Listen_Sock(){
      if(listen(listen_sock,32)<0){
        LOG(FALTA,"listen error!");
      }
      LOG(INFO,"listen success!");
    }

    //得到listen_sock
    int Get_Listen_Sock(){
      return listen_sock;
    }



};
TcpServer*TcpServer::svr=nullptr;
