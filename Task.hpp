
#include"Protocol.hpp"
class Task 
{ 
  private:
    int sock;
   CallBack handler;
  public:
    Task(int _sock=-1):sock(_sock)
    {

    }

    void Process()
    {
      handler.HandlerRequest(sock);
    }

  ~Task()
  {

  }

};
