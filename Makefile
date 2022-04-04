.PHONY:all
all:Server test_cgi
Server:Main.cc
	g++ -o $@ $^ -std=c++11 -lpthread
test_cgi:test_cgi.cc
	g++ -o $@ $^ -std=c++11
.PHONY:clean
clean:
	rm -rf Server test_cgi
	rm wwwroot/test_cgi
