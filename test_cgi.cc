#include"Log.hpp"


void GetMethod(string&query_string)
{
  string method=getenv("METHOD");
  std::cerr<<method<<endl;
  if(method=="GET")
  {
      query_string=getenv("QUERY_STRING");
  }
  else if(method=="POST")
  {

    int content_length=std::atoi(getenv("CONTENT_LENGTH"));
    std::cerr<<"Content_Length:"<<content_length<<endl;
    char ch=0;
    while(content_length--)
    {
      read(0,&ch,1);
      std::cerr<<ch<<endl;
      query_string.push_back(ch);
    }
  }
  else
  {
    std::cerr<<"请求出错"<<endl;
  }
}

int main()
{
  std::cerr<<"你好，cgi！"<<endl;
  string query_string;
  GetMethod(query_string);
  std::cerr<<query_string<<endl;
  cout<<query_string;
  
  return 0;
}
