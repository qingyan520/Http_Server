
#include"HttpServer.hpp"
int main(int argc,char*argv[])
{
  if(argc!=2){
    LOG(FALTA,"./a.out port");
    exit(1);
  }
  int port=atoi(argv[1]);
  HttpServer s(port);
  s.InitHttpServer();
  s.Loop();


  return 0;
}
