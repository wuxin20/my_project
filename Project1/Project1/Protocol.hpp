#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <utility>
#include <unordered_map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include "Util.hpp"

using namespace std;


#define WWW "./wwwroot"
class HttpRequest{
private:
	string request_line;
	string request_header;
	string request_blank;
	string request_body;
private:
	string method;
	string uri;
	string version;
	unordered_map<string, string> header_kv;

	string path;  //资源路径
	string query_string;   //GET上传的参数
	bool cgi;

	int recource_size;
	string suffix;

	bool done;
public:
	HttpRequest() :request_blank("\n"), path(WWW), recource_size(0), cgi(false)//,done(false),suffix(".html")
	{
	}

	bool GetDone()
	{
		return done;
	}

	void SetDone(bool _done)
	{
		done = _done;
	}
	string &GetRequestHeader()
	{
		return request_header;
	}

	string &GetRequestLine()
	{
		return request_line;
	}

	string &GetRequestBody()
	{
		return request_body;
	}

	string GetPath()
	{
		return path;
	}

	string &GetMethod()
	{
		return method;
	}

	string &GetQueryString()
	{
		return query_string;
	}

	bool MethodIsLegal(){
		//GET||POST
		if (method != "GET"&&method != "POST"){
			return false;
		}
		return true;
	}

	string GetSuffix()
	{
		return suffix;
	}

	int GetRecourceSize()
	{
		return recource_size;
	}

	void RequestLineParse(){
		//GET  >index.html  http/1.1
		stringstream ss(request_line);
		ss >> method >> uri >> version;

		Util::StringToUpper(method);

		cout << "method: " << method << endl;
		cout << "uri: " << uri << endl;
		cout << "version: " << version << endl;
		cout << "aaaaaaaa" << endl;
	}

	void RequestHeaderParse()
	{
		vector<string> v;
		Util::TansfromToVector(request_header, v);

		auto it = v.begin();
		for (; it != v.end(); it++){
			string k;
			string v;
			Util::MakeKV(*it, k, v);
			header_kv.insert(make_pair(k, v));
			cout << "key:" << k << endl;
			cout << "value:" << v << endl;
			cout << "lllllll" << endl;
		}

	}


	//1.POST：一定带参数
	//2.GET：如果包含？，则一定带参数
	//3.GET：如果没有？，不带参数
	void UriParse()
	{
		if (method == "POST"){
			cgi = true;
			path += uri;
		}
		else{
			//GET
			size_t pos = uri.find('?');
			if (string::npos == pos){
				path += uri;
			}
			else{
				cgi = true;
				path += uri.substr(0, pos);
				query_string = uri.substr(pos + 1);
			}
		}
		if (path[path.size() - 1] == '/'){
			path += "index.html";
		}
		size_t pos = path.rfind(".");
		if (string::npos == pos){
			suffix = ".html";
		}
		else{
			suffix = path.substr(pos);
		}
		cout << "debug : suffix" << suffix << endl;
	}

	bool IsPathLegal()
	{
		//path="/"
		struct stat st;
		if (stat(path.c_str(), &st) == 0){
			if (S_ISDIR(st.st_mode)){
				path += "/index.html";
			}
			else{
				if ((st.st_mode&S_IXUSR) || \
					(st.st_mode&S_IXUSR) || \
					(st.st_mode&S_IXUSR)){
					cgi = true;
				}
			}
			recource_size = st.st_size;
			return true;
		}
		else{
			return false;
		}
	}

	//yes ->true, no->false
	bool IsNeedRecv()
	{
		return method == "POST";
	}

	int GetContentLength()
	{
		auto it = header_kv.find("Content-Length");
		if (it == header_kv.end()){
			return  -1;
		}
		return Util::StringToInt(it->second);
	}

	bool IsCgi()
	{
		return cgi;
	}

	void Make_404()
	{
		suffix = ".html";
		path = "wwwroot/404.html";
		struct stat st;
		stat(path.c_str(), &st);
		recource_size = st.st_size;
	}

	void ReMakeRequest(int code)
	{
		switch (code){
		case 404:
		case 400:
			Make_404();
			break;
		default:
			break;
		}
	}

	~HttpRequest()
	{}
};


class HttpResponse{
private:
	string response_line;
	string response_header;
	string response_blank;
	string response_body;
private:
	int fd;
	int size;
public:
	HttpResponse() :response_blank("\r\n"), fd(-1)
	{}

	void MakeResponseLine(int code)
	{
		string version = "HTTP/1.0";
		response_line += " ";
		response_line += Util::IntToString(code);
		response_line += " ";
		response_line += Util::CodeToDec(code);
		response_line += "\r\n";
	}


	void MakeResponseHeader(vector<string> &v)
	{
		string connection = "Connection: close";
		v.push_back(connection);
		auto it = v.begin();
		for (; it != v.end(); it++){
			response_header += *it;
			response_header += "\r\n";
		}
	}

	void MakeResponse(HttpRequest *rq, int code, bool cgi)
	{
		MakeResponseLine(code);
		vector<string> v;

		if (cgi){
			//cgi
			string ct = Util::SuffixToType("");
			v.push_back(ct);
			string cl = "Content_Length: ";
			cl += Util::IntToString(response_body.size());
			v.push_back(cl);
			MakeResponseHeader(v);

		}
		else{
			string suffix = rq->GetSuffix();
			size = rq->GetRecourceSize();

			string ct = Util::SuffixToType(suffix);
			v.push_back(ct);
			string cl = "Content-Length: ";
			cl += Util::IntToString(size);
			v.push_back(cl);
			MakeResponseHeader(v);

			string path = rq->GetPath();
			cout << "debug :" << path << endl;
			fd = open(path.c_str(), O_RDONLY);
		}

	}

	string &GetResponseLine()
	{
		return response_line;
	}
	string &GetResponsHeader()
	{
		return response_header;
	}

	string &GetResponseBlank()
	{
		return response_blank;
	}

	string &GetResponseBody()
	{
		return response_body;
	}

	int GetFd()
	{
		return fd;
	}

	int GetRecourceSize()
	{
		return size;
	}


	~HttpResponse()
	{
		if (fd = -1){
			close(fd);
		}
	}

};

class EndPoint{
private:
	int sock;
public:
	EndPoint(int sock_) :sock(sock_)
	{
	}

	int RecvLine(string &line){//\n \r \r\n ->\n
		char c = 'X';
		while (c != '\n'){
			ssize_t s = recv(sock, &c, 1, 0);
			if (s>0){
				if (c == '\r'){
					//\r or \r\n
					if (recv(sock, &c, 1, MSG_PEEK)>0){//窥探
						if (c == '\n'){//\r\n
							recv(sock, &c, 1, 0);
						}
						else{//\r
							c = '\n';
						}
					}
					else{
						c = '\n';
					}
				}
			}
			else if (s == 0){
				c = '\n';
			}
			else{
				c = '\n';
				cout << "recv error: " << s << "sock: " << sock << endl;
			}
			line.push_back(c);
		}
		return line.size();
	}

	void RecvRequestLine(HttpRequest *rq){
		RecvLine(rq->GetRequestLine());   //\n \r \n\r -> \n
	}

	void RecvRequestHeader(HttpRequest *rq)
	{
		string &rh = rq->GetRequestHeader();
		do{
			string line = "";
			RecvLine(line);
			if (line == "\n"){
				break;
			}
			rh += line;
		} while (1);
	}

	void RecvRequestBody(HttpRequest *rq)
	{
		int len = rq->GetContentLength();
		string &body = rq->GetRequestBody();

		char c;
		while (len--){
			if (recv(sock, &c, 1, 0)>0){
				body.push_back(c);
			}
		}
		cout << "aaaaaaallllllll" << endl;
		cout << "body: " << body << endl;
	}

	void SendResponse(HttpResponse *rsp, bool cgi)
	{

		string &response_line = rsp->GetResponseLine();
		string &response_header = rsp->GetResponsHeader();
		string &response_blank = rsp->GetResponseBlank();
		send(sock, response_line.c_str(), response_line.size(), 0);
		send(sock, response_header.c_str(), response_header.size(), 0);
		send(sock, response_blank.c_str(), response_blank.size(), 0);
		if (cgi){
			string &response_body = rsp->GetResponseBody();
			send(sock, response_body.c_str(), response_body.size(), 0);
		}
		else{
			//non cgi ->GET, no args;          
			int fd = rsp->GetFd();
			int size = rsp->GetRecourceSize();

			sendfile(sock, fd, NULL, size);
		}
	}

	void ClearRequest(HttpRequest *rq)
	{
		if (rq->GetDone()){
			return;
		}
		if ((rq->GetRequestHeader()).empty()){
			RecvRequestHeader(rq);
		}
		if (rq->IsNeedRecv()){
			if ((rq->GetRequestBody()).empty()){
				RecvRequestBody(rq);
			}
		}
		rq->SetDone(true);
	}

	~EndPoint()
	{
		close(sock);
	}
};


class Entry{
public:
	static int ProcessCgi(HttpRequest *rq, HttpResponse *rsp)
	{
		int code = 200;
		string path = rq->GetPath();

		string &body = rq->GetRequestBody();    //null   ->GET
		string &method = rq->GetMethod();
		string &query_string = rq->GetQueryString();
		int content_length = rq->GetContentLength();


		string cont_len_env = "CONTENT_LENGTH=";
		string method_env = "METHOD=";
		method_env += method;


		string query_string_env = "QUERY_STRING=";
		query_string_env += query_string;

		string &rsp_body = rsp->GetResponseBody();

		//CGI程序角度，子进程
		int input[2] = { 0 };
		int output[2] = { 0 };
		pipe(input);
		pipe(output);

		pid_t id = fork();
		if (id<0){
			//TODO
			code = 500;
		}
		else if (id == 0){//子进程
			close(input[1]);
			close(output[0]);

			//重定向
			dup2(input[0], 0);
			dup2(output[1], 1);

			putenv((char*)method_env.c_str());

			if (method == "POST"){
				cont_len_env += Util::IntToString(content_length);
				putenv((char*)cont_len_env.c_str());
			}
			else if (method == "GET"){
				putenv((char*)query_string_env.c_str());
				cout << "GET:Query_string: " << query_string_env << endl;
			}
			else{
				//TODO
			}
			//如果方法是GET-> query_string->new
			//如果方法是POST->  body->stdin,stdout
			execl(path.c_str(), path.c_str(), nullptr);
			exit(1);
		}
		else{//父进程
			close(input[0]);
			close(output[1]);
			if (method == "POST"){
				auto it = body.begin();
				for (; it != body.end(); it++){
					char c = *it;
					write(input[1], &c, 1);
				}
			}

			char c;
			while (read(output[0], &c, 1)>0){
				rsp_body.push_back(c);
			}
			waitpid(id, NULL, 0);
		}
		return code;
	}

	//1.通过endpoint读取请求，并且构建request
	//2.分析request，得出具体细节
	//3.构建response
	//4.通过endpoint，返回response，最终完成请求/响应，close(sock);
	//5.备注：上述步骤，可能彼此交叉进行
	static void *HandlerRequest(void *args){
		int code = 200;
		int *p = (int*)args;
		int sock = *p;
		EndPoint *ep = new EndPoint(sock);
		HttpRequest *rq = new HttpRequest();
		HttpResponse *rsp = new HttpResponse();

		ep->RecvRequestLine(rq);
		rq->RequestLineParse();
		if (!rq->MethodIsLegal()){
			//非法方法
			//TODO
			//code=400;
			goto end;
		}

		ep->RecvRequestHeader(rq);
		rq->RequestHeaderParse();
		if (rq->IsNeedRecv()){
			ep->RecvRequestBody(rq);
		}
		rq->SetDone(true);

		rq->UriParse();
		if (!rq->IsPathLegal()){
			//TODO
			code = 404;
			goto end;
		}
		//mrthod path ,query_string,body,version,cgi
		if (rq->IsCgi()){
			//DO CGI
			code = ProcessCgi(rq, rsp);
			if (code == 200){
				rsp->MakeResponse(rq, code, true);
				ep->SendResponse(rsp, true);
			}
		}
		else{//GET,no args
			rsp->MakeResponse(rq, code, false);
			ep->SendResponse(rsp, false);
		}

	end:
		if (code != 200){
			ep->ClearRequest(rq);
			rq->ReMakeRequest(code);
			rsp->MakeResponse(rq, code, false);
			ep->SendResponse(rsp, false);
		}
		delete rq;
		delete rsp;
		delete ep;
		delete p;

	}
};




