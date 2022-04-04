#include"Log.hpp"
class Util{
  public:
     static   int GetLine(int fd,string&line)
    {
     //得到每一行，处理'\n','\r\n','\r'三种情况 
      char ch=0;
      int size=0;
      while(ch!='\n')
      {
         size=recv(fd,&ch,1,0);
         if(size<0)
           return -1;

         if(ch=='\r'){
            size=recv(fd,&ch,1,MSG_PEEK);
            if(size<0)
              return -1;
            if(ch=='\n'){
            size=recv(fd,&ch,1,0);
            if(size<0)
              return -1;
            }
            else{
              ch='\n';
            }
         }
         line.push_back(ch);
     }

    return line.size();
  }


     //切分字符串
     static void CutString(string target,string&key,string&val,string sep){

       auto pos=target.find(sep);

       if(pos!=std::string::npos){
          key=target.substr(0,pos);
          val=target.substr(pos+sep.size());
       }
       else{
         LOG(WARNING,"CutString Error!");
       }
     }


     //无论method，全部转化为大写
     static void UpplerMethod(string&method){
       for(unsigned long i=0;i<method.size();i++){
         if(method[i]>='a'&&method[i]<='z'){
           method[i]-=32;
         }
       }
     }
};
