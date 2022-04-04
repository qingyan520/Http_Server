#include"Log.hpp"
#include"Util.hpp"

#define SEP ": "
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#define PAGE_404 "404.html"
#define PAGE_500 "500.html"
#define OK 200
#define NOT_FOUND 404
#define SERVER_ERROR 500
static string Code2Desc(int code)
{
  switch(code)
  {
    case 200:
      return "OK";
    case 404:
      return "Not_Found";
  }
  return "";

}


//后缀转化
static string Suffix2Desc(const string&str)
{
  static unordered_map<string,string>su={
      {".html","text/html"},
      {".gif","image/gif"},
      {".jpg","application/x-jpg"},
      {".png","application/x-png"},
      {".css","text/css"},
      {".ico","image/x-icon"},
      {".js","application/x-javascript"},
      {".mp3","video/mp3"},
      {".txt","text/plain"},
      {".svg","text/xml"},
      {".woff","font/woff"},
      {".woff2","font/woff2"},
      {".ttf","application/x-font-ttf"},
      {".tif/tiff","image/tiff"},
      {".mp4","audio/mp4"}
  };
  auto iter=su.find(str);
  if(iter!=su.end())
  {
    return iter->second;
  }
  return "text/html";

}


//定制协议
class HttpRequest{
  
  public:
    string request_line;//请求行
    vector<string>request_header;//请求报头
    string request_blank;//空行
    string request_body;//请求正文

    string method;//请求方法
    string uri;//请求路径
    string version;//http版本
    unordered_map<string,string>header_kv;//将请求报头设置为kv模型
    int Content_Length;
    string path;
    string query_string;
    bool cgi=false;
    string suffix=".html";
    int size=0;

};


//http回应
class HttpReponse{
  public:
    string status_line;
    vector<string>response_header;
    string blank;
    string reponse_body;
    int status_code=OK;

    int fd;
    int size;
};






class EndPoint{
  private:
    int fd;//主要用于通信
    HttpRequest httprequest;
    HttpReponse httpresponse;
    bool stop=false;


    //读取请求行
    bool RecvRequestLine(){

        if((Util::GetLine(fd,httprequest.request_line)<0))
        {
          stop=true;
          return true;
        }
        httprequest.request_line.pop_back();
        cout<<httprequest.request_line<<endl;
        LOG(INFO,"RecvRequestLine Success!");
        return false;
    }

    //读取请求报头
    bool RecvRequestHeader(){
      string line;
      while(line!="\n"){
        line.clear();
        
       int ret= Util::GetLine(fd,line);
       if(ret<0)
       {
         stop=true;
         return true;
       }
        if(line!="\n"){
          line.pop_back();
          httprequest.request_header.push_back(line);
        }
        //cout<<line<<endl;
      }

      if(line=="\n"){
        httprequest.request_blank=line;
      }
      LOG(INFO,"RecvRequestHeader Success!");
      return false;
    }

    //拆分请求行
    void ParseRequestLine(){
        stringstream ss(httprequest.request_line);
        ss>>httprequest.method>>httprequest.uri>>httprequest.version;
        //将方法统一转化为大写
        Util::UpplerMethod(httprequest.method);

        LOG(INFO,"ParseRequestLine Success!");
    }

    //拆分请求报头
    void ParseRequestHeader(){
      for(auto iter:httprequest.request_header){
        string key;
        string val;
        Util::CutString(iter,key,val,SEP);
        httprequest.header_kv.insert(make_pair(key,val));
        //cout<<key<<":"<<val<<endl;
      }
      LOG(INFO,"ParseRequestHeader Success!");
    }

    //判断是否需要进行读取请求正文
    bool IsNeedRecvRequestBody(){
      auto method=httprequest.method;
      if(method=="POST"){
        //寻找Content-Length是否存在
        auto iter=httprequest.header_kv.find("Content-Length");
        if(iter!=httprequest.header_kv.end())
        {
          httprequest.Content_Length=std::stoi(iter->second);

          return true;
        }
        else{
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    //读取正文
    bool RecvRequestBody(){
      if(IsNeedRecvRequestBody()){
        int content_length=httprequest.Content_Length;
        char ch='x';
        while(content_length--){
          int ret=recv(fd,&ch,1,0);
          if(ret<0)
          {
            stop=true;
            return true;
          }
          httprequest.request_body.push_back(ch);
        }

        cout<<"------------------------------------------------------------------------------"<<httprequest.request_body<<endl;
      }
      

      LOG(INFO,"RecvRequestBody Success!");
      return false;

    }
    

    int ProcessNonCgi()
    {
      httpresponse.fd=open(httprequest.path.c_str(),O_RDONLY);
      if(httpresponse.fd>=0)
      { 
        
        LOG(INFO,"ProcessNonCgi Success!");
        return OK;
      }

      return NOT_FOUND;

    }

    int ProcessCgi()
    {
      //处理是可执行程序的问题
      //pid_t id=fork();
      
      //建立双向管道用于父子进程间通信
      //子进程进行进程程序替换
      int output[2];
      int input[2];
      if(pipe(output)<0)
        return SERVER_ERROR;
      if(pipe(input)<0)
        return SERVER_ERROR;
      
      string &method=httprequest.method;
      string METHOD="METHOD=";
      METHOD+=method;
      pid_t id=fork();
      if(id==0)//子进程
      {
        close(output[1]);//留下0进行读取
        close(input[0]);//留下1进行书写
        
      

        //利用环境变量导入METHOD，然后在cgi程序中分析到底是什么方法导入的
        //如果是get方法，那么就query_string中填着参数，利用环境变量发送这个消息
        //如果是post方法，就在http_response.reponse_body中，我们利用管道进行读取

        putenv((char*)METHOD.c_str());
      
        if(method=="GET")
        {
          //说明query_string中是我们想要的内容
          //继续利用环境变量进行导入
          string QUERY_STRING="QUERY_STRING=";
          QUERY_STRING+=httprequest.query_string;
          putenv((char*)QUERY_STRING.c_str());
        }

        else if(method=="POST")
        {
          //post时将content-length发送过去
          string CONTENT_LEGHTH_ENV="CONTENT_LENGTH=";
          CONTENT_LEGHTH_ENV+=std::to_string(httprequest.Content_Length);
          cout<<CONTENT_LEGHTH_ENV<<endl;
          cout<<"pid -------------------------Length:"<<httprequest.Content_Length<<endl;
          putenv((char*)CONTENT_LEGHTH_ENV.c_str());

        }
   
        
         cout<<"bin: "<<httprequest.path<<endl;
        dup2(output[0],0);//将标准输入重定向到output[0]
         dup2(input[1],1);//将标准输出重定向到input[1]
        

        execl(httprequest.path.c_str(),httprequest.path.c_str(),nullptr);
        
        close(output[0]);
        close(input[1]);
        exit(1);
      }

      else if(id>0)//父进程
      {
        close(output[0]);//1用来向子进程写入
        close(input[1]);//0用来读取子进程

        //如果是post就有父亲利用管道进行写入
        if(httprequest.method=="POST")
        {
            unsigned long total=0;
            ssize_t size=0;
            cout<<"父进程写入request.request_body!!!!!!!"<<endl;
            const char*str=httprequest.request_body.c_str();
          cout<<"父进程写入的内容为:"<<str<<endl;
          cout<<"size="<<httprequest.request_body.size()<<endl;
            while(total<httprequest.request_body.size())
            {
                cout<<"父进程传输数据！！！！！！"<<endl;
                size=write(output[1],str+total,httprequest.request_body.size()-total);
                
                cout<<"父进程传输数据之后--------------------"<<endl;
                if(size>0)
                {
                  cout<<"父进程写入成功："<<size<<endl;
                  total+=size;
                }
                else
                {

                  LOG(WNRING,"父进程写入失败！！！！！！！！！！！！！！！！！！！！");
                }

                cout<<"父进程写入数据完毕"<<endl;
            }

        }
        
            
        //父进程读取子进程发送过来的内容
        char ch=0;
        while(read(input[0],&ch,1)>0)
        {
            httpresponse.reponse_body.push_back(ch);
        }
        httpresponse.size=httpresponse.reponse_body.size();
        cout<<"父进程接受的大小为httpresponse.size="<<httpresponse.size<<endl;
        cout<<"父进程读取到的为：" <<httpresponse.reponse_body<<endl; 
  
          
      
          
          


        int status=0;
        pid_t ret=waitpid(id,&status,0);
        return OK;
        if(ret==id)
        {
          if(WIFEXITED(status))
          {
          
            return  OK;
          }
          return SERVER_ERROR;
        }
    
        close(output[1]);
        close(input[0]);

      }
      else
      {
        return SERVER_ERROR;
      }
      return OK;
    }


    //构建OK的response
    void HanderOkResponse()
    {
      string line="Content-Type: ";
      line+=Suffix2Desc(httprequest.suffix);
      line+="\r\n";

      httpresponse.response_header.push_back(line);

      //注意两种方式的size,GET方式为httprequest.size,而POST方法回应正文在repsonse.request_body中，需要返回这个body,所以大小为body的大小
      line="Content-Length: ";
      if(httprequest.cgi)
      {
        line+=std::to_string(httpresponse.size);
      }
      else
      {
        cout<<"get 方法"<<endl;
        line+=std::to_string(httprequest.size);
      }
      line+="\r\n";

      httpresponse.response_header.push_back(line);
      
      httpresponse.blank="\r\n";
    }

   
    void HanderError(string PAGE)
    {
        //走到这里说明资源不存在或者cgi处理出错，那么我们统一发送错误页面，前提保证错误页面一定要存在
        //并且将cgi设置为false
        httprequest.cgi=false;
        //打开错误页面
        //注意文件路径
        string path=WEB_ROOT;
        path+="/";
        path+=PAGE;
    
      httpresponse.fd=open(path.c_str(),O_RDONLY);

      if(httpresponse.fd>=0)
        {
          struct stat st;
          stat(path.c_str(),&st);
          string line="Content-Type: text/html\r\n";
          httpresponse.response_header.push_back(line);
          line="Content-Length: ";
          httprequest.size=st.st_size;
          line+=std::to_string(httprequest.size);
          line+="\r\n";
          httpresponse.response_header.push_back(line);
          httpresponse.blank="\r\n";
        }


    }


    //构建response
    void BuiltHttpResponseHelper()
    {
      auto code=httpresponse.status_code;
      
      //构建回复行
      string line=HTTP_VERSION;
      line+=" ";
      line+=std::to_string(code);
      line+=" ";
      line+=Code2Desc(code);
      line+="\r\n";
      httpresponse.status_line=line;

      switch(code)
      {
        case 200:
          HanderOkResponse();
          break;
        case 404:
          HanderError(PAGE_404);
          break;
        case 500:
          HanderError(PAGE_500);
          break;
        default:
          break;
      }

    }


  
  public:
    EndPoint(int cfd):fd(cfd){

    }
    
    

    //接受http请求，接受请求行，将请求行进行分割为method,path,version,读取header,然后切分为kv结构
    //判读是否需要进行读取正文，如果需要则读取正文
    //
    void RecvRequest(){
      LOG(INFO,"REcvRequest Begin!");
      if(!RecvRequestLine()&&(!RecvRequestHeader()))
      {
          ParseRequestLine();
          ParseRequestHeader();
          LOG(INFO,"RecvRequestLine && RecvRequestHeader Success!");
          LOG(INFO,"ParseRequestLine && ParseRequestHeader Success!");
          if(RecvRequestBody())
          {
              LOG(WAINING,"RecvRequestBody Eeeor!");
          }
          else
          {
            LOG(INFO,"RecvRequestBody Success!");
          }
      }
      else
      {
        LOG(WAINING,"RecvRequestLine Or RecvRequestBody Error!");
      }
  
      LOG(INFO,"RecvRequest End!");
    }
    



    //判断请求是否合理
    //分析http请求
    //1.path为wwwroot+uri,代表位于当前就进程文件目录下的一个文件，如果最后一个为/，后面默认+index.html
    //2.判断这是一个文件还是一个文件夹，如果是一个文件夹，我们默认也给他+/index.html
    //3.如果不是文件夹，那么就可能是可执行程序了，
    void BuiltHttpResponse()
    {
      string path;
    unsigned long  found=0;
      auto &code=httpresponse.status_code;
  
      if(httprequest.method!="GET"&&httprequest.method!="POST"){
        LOG(WARNING,"method is not right!");
        code=NOT_FOUND;
        goto End;
      }


      //判断是GET方法还是POST方法
      if(httprequest.method=="GET")
      {
        if(httprequest.uri.find("?")!=string::npos)
        {
          Util::CutString(httprequest.uri,httprequest.path,httprequest.query_string,"?");
          httprequest.cgi=true;   
        }
        else
        {
           httprequest.path=httprequest.uri; 
        }
      }
      else if(httprequest.method=="POST")
      {
        httprequest.cgi=true;
        httprequest.path=httprequest.uri;
      }
      else
      {
        //do_nothing
      }



      //判断是否以/结尾
      if(httprequest.path[httprequest.path.size()-1]=='/')
      {
        httprequest.path+=HOME_PAGE;
      }

      path=WEB_ROOT+httprequest.path;
      httprequest.path=path;

      //path已经处理好了，现在就该判断是否为目录
      struct stat st;
      //stat函数，如果返回值为0代表该资源存在
      if(stat(httprequest.path.c_str(),&st)==0)
      {
        //说明该资源存在
        //判断是否为目录
        if(S_ISDIR(st.st_mode))
        {
          httprequest.path+="/";
          httprequest.path+=HOME_PAGE;

          if(stat(httprequest.path.c_str(),&st)<0)
          {
            string info=httprequest.path;
            info+="Not_Found";
            LOG(WAINING,info);
            code=NOT_FOUND;
            goto End;
          }
          else
          {
            httprequest.size=st.st_size;
          }
        }
        else if((st.st_mode&S_IXUSR)||(st.st_mode&S_IXGRP)||(st.st_mode&S_IXOTH))
        {
          //说明该文件为可执行程序
          httprequest.cgi=true;
        } 
       // stat(httprequest.path.c_str(),&st);
       // size=st.st_size;
        else
        {
          code=OK;
          httprequest.size=st.st_size;
        }
      }
      else
      {
        //走到这里代表资源不存在
        string info=httprequest.path;
        info+=" Not_Found";
        LOG(WARNING,info);
        code=NOT_FOUND;
        goto End;
      }
      
      found=httprequest.path.rfind(".");
      if(found!=std::string::npos)
      {
        httprequest.suffix=httprequest.path.substr(found);
      }
      else
      {
        httprequest.suffix=".html";
      }
      if(httprequest.cgi)
      {
        //特殊处理cgi
        cout<<"ProcessCgi :httprequest.cgi:"<<httprequest.cgi<<endl;
        code=ProcessCgi();
      }
      else
      {
        //只进行简单的网页处理
        code=ProcessNonCgi();
      }

End:
      cout<<"httprequest.cgi:"<<httprequest.cgi<<endl;
      cout<<"httpresponse.status_code:"<<httpresponse.status_code<<endl;
      BuiltHttpResponseHelper();


    }


    void SendHttpResponse(){
      send(fd,httpresponse.status_line.c_str(),httpresponse.status_line.size(),0);

      for(auto iter:httpresponse.response_header)
      {
        send(fd,iter.c_str(),iter.size(),0);
      }
      send(fd,httpresponse.blank.c_str(),httpresponse.blank.size(),0);
      
      if(httprequest.cgi)
      {
        //发送cgi解析的内容
        int size=0;
        int total=0;
        const char*str=httpresponse.reponse_body.c_str();

        while(total<httpresponse.size)
        {
          size=send(fd,str+total,httpresponse.size-total,0);
          total+=size;
        }
        cout<<"Send Cgi_File_Fd:"<<fd<<endl;
        cout<<"Send File_Size:"<<total<<endl;
        close(httpresponse.fd);
        LOG(INFO,"Cgi Send Success!");
      }
      else
      {
        sendfile(fd,httpresponse.fd,nullptr,httprequest.size);
        cout<<"Send File_Fd:"<<httpresponse.fd<<endl;
        cout<<"Send_File_Size:"<<httprequest.size<<endl;

        LOG(INFO,"Send File Success!");
        close(httpresponse.fd);
      }
      LOG(INFO,"SendHttpResponse Success!");
    }



    //判断是否进行
    bool IsStop()
    {
      return stop;
    }







    ~EndPoint(){
      if(fd>=0)
      {
        close(fd);
      }

    }

};












class CallBack{

  public:
    CallBack()
    {

    }
    void HandlerRequest(int fd){
      LOG(INFO,"=======================================HandlerRequest Begin!================================");
    char buf[4096]={0};
    struct sockaddr_in peer;
    socklen_t len=sizeof(peer);
     recvfrom(fd,buf,sizeof(buf),MSG_PEEK,(struct sockaddr*)&peer,&len);
    string ip=inet_ntoa(peer.sin_addr);
    int _port=ntohs(peer.sin_port);
    string info="--------------ip:";
    info+=ip;
    info+=" port:";
    info+=std::to_string(_port);
    info+="---------------------";
    LOG(INFO,info);
      
      EndPoint*ep=new EndPoint(fd);
      ep->RecvRequest();
      if(!ep->IsStop()){
        ep->BuiltHttpResponse();
        ep->SendHttpResponse();
        LOG(INFO,"BuiltHttpResponse && SendHttpResponse Success!");
      }
      delete ep;

    }

    void operator()(int sock)
    {
        HandlerRequest(sock);
    }
};
