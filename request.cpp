// hostname port request string

// pass it a string when we create it and it parses those out instead of doing it in main

#include <stdlib.h>
#include <iostream>
#include "string.h"

using namespace std;

class Request{
	private:
		string hostName;
		string request;
		int port;
		
		string parseHost(string s){
			return s;
		}
		int parsePort(string s){
			return -1;

		}
		string parseRequest(string s){
			return s;

		}
	public:
		Request(string s){
			hostName = parseHost(s);
			port = parsePort(s);
			request = parseRequest(s);
		}

};