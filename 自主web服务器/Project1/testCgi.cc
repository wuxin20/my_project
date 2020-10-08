#include <iostream>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include "Util.hpp"



using namespace std;

int GetData(string &str)
{
	size_t pos = str.find('=');
	if (string::npos != pos){
		return  Util::StringToInt(str.substr(pos + 1));
	}
}

int main()
{
	string args;
	string method = getenv("METHOD");
	if (method == "GET"){
		args = getenv("QUERY_STRING");
	}
	else if (method == "POST"){
		//POST
		string cl = getenv("CONTENT_LENGTH");
		int content_length = Util::StringToInt(cl);
		char c;
		while (content_length--){
			read(0, &c, 1);
			args.push_back(c);
		}
	}
	else{
		//bug

	}

	size_t pos = args.find('&');
	if (string::npos != pos){
		string first = args.substr(0, pos);
		string second = args.substr(pos + 1);

		int data1 = GetData(first);
		int data2 = GetData(second);

		cout << data1 << "+" << data2 << "=" << data1 + data2 << endl;
		cout << data1 << "-" << data2 << "-" << data1 + data2 << endl;
		cout << data1 << "*" << data2 << "*" << data1 + data2 << endl;
		cout << data1 << "/" << data2 << "/" << data1 + data2 << endl;

	}
	cout << "method: " << method << endl;
	cout << "args: " << args << endl;
	cerr << "cerr: method: " << method << endl;
	cerr << "cerr: args: " << args << endl;
	return 0;
}

