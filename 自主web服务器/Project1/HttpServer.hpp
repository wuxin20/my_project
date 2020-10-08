#pragma once

#include <iostream>
#include <pthread.h>
#include "Protocol.hpp"
#include "Util.hpp"
#include "ThreadPool.hpp"


using namespace std;



class Sock{
private:
	int sock;
	int prot;

public:
	Sock(const int &prot_) :prot(prot_), sock(-1)
	{}

	//创建
	void Socket(){
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock<0){
			cerr << "socket error" << endl;
			exit(2);
		}
		int opt = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}

	//绑定
	void Bind(){
		struct sockaddr_in local;
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		local.sin_port = htons(prot);

		if (bind(sock, (struct sockaddr*)&local, sizeof(local))<0){
			cerr << "bind error" << endl;
			exit(3);
		}
	}

	//监听
	void Listen(){
		const int backlog = 10;
		if (listen(sock, backlog)<0){
			cerr << "listen error" << endl;
			exit(4);
		}
	}

	//建立连接
	int Accept(){
		struct sockaddr_in peer;
		socklen_t len = sizeof(peer);
		int fd = accept(sock, (struct sockaddr*)&peer, &len);
		if (fd<0){
			cerr << "accept error" << endl;
			return -1;
		}
		cout << "get a new link ... done" << endl;
		return fd;
	}

	~Sock(){
		if (sock >= 0){
			close(sock);
		}
	}
};


#define DEFAULT_PORT 8080

class HttpServer{
private:
	Sock sock;
	//ThreadPool *tp;
public:
	HttpServer(int port = DEFAULT_PORT) :sock(port)//,tp(nullptr)
	{}

	void InitHttpServer(){
		sock.Socket();
		sock.Bind();
		sock.Listen();
		//tp=new ThreadPool(8);
		//tp->InitThreadPool();
	}

	void Start(){
		for (;;){
			int sock_ = sock.Accept();
			if (sock_ >= 0){
				pthread_t tid;
				int *p = new int(sock_);
				//Task t(p,Entry::HandlerRequest);
				//tp->PushTask(t);
				pthread_create(&tid, nullptr, Entry::HandlerRequest, p);
			}
		}
	}
};

