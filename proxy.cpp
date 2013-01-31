#include <iostream>
#include "string.h"
using namespace std;

// Networking includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Program constants
const int NUM_ARGS = 3;
const int HOST_INDEX = 1;
const int PORT_INDEX = 2;
const bool PRODUCTION = false;

// Defaults
char * DEFAULT_HOST = "www.google.com";
char * DEFAULT_PORT = "80";

// Function prototypes
bool validCmdArgs(int argc, char* argv[]);
char * getHost(char * args[]);
char * getPort(char * args[]);
int hostnameToIp(char* host, char* ip);

int main(int argc, char * argv[]) {
  // Get user name and host name
  char * host, * port;
  bool args = validCmdArgs(argc, argv);
  if (!args && PRODUCTION) {
    cerr << "Invalid command line args specified." << endl;
    cerr << "Usage: \"./proxy <hostname> <port>\"" << endl;
    return -1;
  } else if (!args && !PRODUCTION) {
    host = DEFAULT_HOST;
    port = DEFAULT_PORT;
  } else {
    host = getHost(argv);
    port = getPort(argv);
  }

  cout << "Requesting " << host << " at port " << port << endl;
  char ip[20];
  hostnameToIp(host, ip);
  cout << ip << endl;
  return 0;
}

bool validCmdArgs(int argc, char* argv[]) {
  return argc == NUM_ARGS;
}

char * getHost(char * args[]) {
  return args[HOST_INDEX];
}

char * getPort(char * args[]) {
  return args[PORT_INDEX];
}

int hostnameToIp(char* host, char* ip) {
  struct hostent * he = gethostbyname(host);
  struct in_addr ** addr_list;
  if (he == NULL) {
    cerr << "Hostname cannot be resolved." << endl;
    return 1;
  }
  addr_list = (struct in_addr **) he->h_addr_list;
  strcpy(ip, inet_ntoa(*addr_list[0]));
  return 1;
}

