#ifndef REQUEST
#define REQUEST

#include <stdlib.h>
#include <iostream>
#include "string.h"

using namespace std;

class Request{
	private:
		int socket;

		void parseAll(string s){
			int i = 0;

			string host="";
			string path="";
			while(i != s.length() && s[i] != 'h'){
				i++;
			}

			while(i != s.length() && s[i] != '/'){
				i++;
			}
			i++;
			while(i != s.length() && s[i] != '/'){
				i++;
			}
			i++;
			while(i != s.length() && s[i] != '/'){
				host += s[i];
				i++;
			}
			while(i != s.length() && s[i] != ' '){
				path+= s[i];
				i++;
			}
			hostName = host;
			pathName = path;
		}
	public:
		string hostName;
		string pathName;
		Request(string s, int sock){
			socket = sock;
			parseAll(s);
		}

		int getSock(){
			return socket;
		}
};

#endif

