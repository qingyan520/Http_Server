#!/bin/bash
make clean
make

cp test_cgi wwwroot/



sudo kill -9 $(lsof -i:8080 -t)
netstat -ltp

nohup ./Server 8080&
