### 高性能web服务器项目

#### 项目简介

> 本项目旨在利用c/c++,实现简单的web服务器，可以进行静态网页的读取，cgi机制的响应，通过本项目你可以更加深入的理解http协议，理解网络传输细节



#### 开发环境

> Centos 7 vim /g++/gdb +c/c++



#### 技术特点

> 网络编程(TCP/IP协议，socket编程,http协议)

> 多线程技术

> cgi机制

> 线程池

#### 项目使用与演示

##### 项目使用

```cpp
git clone https://github.com/qingyan520/Http_Server.git
```

```shell
cd Http_Server
```

```
./re.sh
```

> 只需要上面这三步便可以将这个项目部署到自己的服务器上，当然，你也可以替换自己的网页到wwwroot目录下

```shell
rm -rf wwwroot
```

```shell
mkdir wwwroot
```

```shell
cd wwwroot
```

```shell
rz -e 文件或者将文件托转到xshell中 
```

```shell
cd ..
```

```shell
./re.sh    #即可运行
```

##### 项目演示

###### 静态网页展示

![项目演示](https://raw.githubusercontent.com/qingyan520/Cloud_img/master/img/%E9%A1%B9%E7%9B%AE%E6%BC%94%E7%A4%BA.gif)

###### cgi机制演示

![cgi项目演示](https://raw.githubusercontent.com/qingyan520/Cloud_img/master/img/cgi%E9%A1%B9%E7%9B%AE%E6%BC%94%E7%A4%BA.gif)



#### 项目步骤概述

> 创建套接字
>
> 监听
>
> 绑定
>
> 监听
>
> accept()
>
> 引入多线程/线程池处理
>
> 读取http响应(一行一行读取)
>
> 读取请求行
>
> 读取请求报头
>
> 将请求行拆分为method ，uri和version
>
> 利用map将请求报头差分为[key,value]形式
>
> 判读是否读取请求正文：判断方法是否为POST(注意将方法全部转化为大写)，是POST，而后判断map[Content-Length]是否存在， 若上面两个要求都满足，则读取请求正文
>
> 将http请求读取完毕之后我们就需要进行http解析
>
> 首先判断请求方法是否合理，本项目只使用了GET/POST方法，如其他方法本项目一律设置为不合法
>
> 如果为GET方法，则判断是否在uri中带参数了，如果uri中含有？，那么？后面的就是参数，我们就截取？前面作为要访问资源的路径，？后面作为参数
>
> 如果为POST方法，那么uri就是要访问资源的路径了，
>
> 判断该路径的资源存不存在，如果是文件夹，后接index.html拼接路径，然后判断是否存在，若不存在
>
> 如果是二进制可以文件：引入cgi机制执行可执行文件处理数据
>
> 如果是文件就正常处理
>
> 处理cgi机制
>
> fork父子进程，通过管道或者环境变量将数据发送给子进程进行处理，子进程将处理玩的数据通过管道交给父进程
>
> 非cgi机制
>
> 正常处理
>
> 根据上面操作的状态码构建http响应，构建响应行，响应报头，响应正文，如果是cgi机制就发送响应正文，非cgi机制就发送文件
>
> 到此，简单的http响应就处理完毕了
>

> 
>
> 读取出错处理
>
> 在读取报头的时候会出现客户端关闭，客户端刷新等场景导致读取出错，在这种情况下我们就不进行后续操作了
>
> 写入出错处理
>
> 在server向客户端发送数据的时候，会出现客户端断开连接等导致写入出错的情况，此时会发出一个sigpipe的信号终止服务器进程，导致服务器奔溃，这时我们就忽略sigpipe信号
>

#### 项目步骤详细说明

##### [1.TcpServer类的创建](https://github.com/qingyan520/Http_Server/blob/main/项目文档/1.TcpServer类的创建.md)

##### [2.Log.hpp日志的创建](https://github.com/qingyan520/Http_Server/blob/main/项目文档/2.Log.hpp日志的创建.md)

##### [3.TcpServer类的创建以及多线程的引入](https://github.com/qingyan520/Http_Server/blob/main/项目文档/3.TcpServer类的创建以及多线程的引入.md)

##### [4.Util工具类的创建](https://github.com/qingyan520/Http_Server/blob/main/项目文档/4.Util工具类的创建.md)

##### [5.http请求的读取](https://github.com/qingyan520/Http_Server/blob/main/项目文档/5.http请求的读取.md)

##### [6.http请求的分析](https://github.com/qingyan520/Http_Server/blob/main/项目文档/6.http请求的分析.md)

##### [7.cgi机制的引入与实现](https://github.com/qingyan520/Http_Server/blob/main/项目文档/7.cgi机制的引入与实现.md)

##### [8.http响应的构建](https://github.com/qingyan520/Http_Server/blob/main/项目文档/8.http响应的构建.md)

##### [9.线程池的引入](https://github.com/qingyan520/Http_Server/blob/main/项目文档/9.线程池的引入.md)

