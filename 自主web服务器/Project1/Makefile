.PHONY:all
all:HttpServer testCgi
testCgi:testCgi.cc
	g++ -o $@ $^
HttpServer:HttpServer.cc
	g++ -g -o $@ $^ -std=c++11 -lpthread -g

.PHONY:clean
clean:
	rm -f HttpServer testCgi

