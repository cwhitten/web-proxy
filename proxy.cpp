#include <iostream>
#include "string.h"
#include <stdlib.h>
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
char * DEFAULT_REQUEST = "GET / HTTP/1.0\r\n\r\n";

// Function prototypes
bool validCmdArgs(int argc, char* argv[]);
char * getHost(char * args[]);
char * getPort(char * args[]);
unsigned short getPort(char* p);
int getSocket();
unsigned long getServer(char* addr);
void connectSocket(int sock, char* addr, char* port);
void bindSocket(int sock, char* servPort);
void listenSocket(int sock, int numPending);
int acceptSocket(int sock);
void hostnameToIp(char* host, char* ip);
void sendSock(int sock, char * msg);
void recvSock(int sock, char * buf);

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

  char ip[20];
  hostnameToIp(host, ip);
  cout << "Trying " << ip << "..." << endl;

  int sock = getSocket();
  connectSocket(sock, ip, port);
  cout << "Connected to " << ip << "." << endl;

  // Send and receive
  sendSock(sock, DEFAULT_REQUEST); 
  cout << "Request has been made." << endl;

  close(sock);

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

void hostnameToIp(char* host, char* ip) {
  struct hostent * he = gethostbyname(host);
  struct in_addr ** addr_list;
  if (he == NULL) {
    cerr << "Hostname cannot be resolved." << endl;
    exit(-1);
  }
  addr_list = (struct in_addr **) he->h_addr_list;
  strcpy(ip, inet_ntoa(*addr_list[0]));
}

// Casts a char string to an unsigned short for usage
// as a port in networking functions
unsigned short getPort(char* p) {
  return (short) atoi(p);
}

// Wrapped socket() function that handles errors internally
int getSocket() {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    cerr << "Error with socket process." << endl;
    exit(-1);
  }
  return sock;
}

// Gets server IP address
unsigned long getServer(char* addr) {
  unsigned long servIP;
  int status = inet_pton(AF_INET, addr, &servIP);
  if (status <= 0) {
    cerr << "Error Occured with server IP process." << endl;
    exit(-1);
  }
  return servIP;
}

// Goes through the logic of connecting a socket
void connectSocket(int sock, char* addr, char* port) {
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = getServer(addr);
  servAddr.sin_port = htons(getPort(port));;

  int status = connect(sock, (struct sockaddr *) &servAddr,
      sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with connect process." << endl;
    exit(-1);
  }
}


// Goes through logic and error checking of binding a socket
void bindSocket(int sock, char* servPort) {
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(getPort(servPort));

  int status = bind(sock, (struct sockaddr*) &servAddr,
                            sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with bind" << endl;
    exit(-1);
  }
}

// Goes through logic of setting port to listen
void listenSocket(int sock, int numPending) {
  int status = listen(sock, numPending);
  if (status < 0) {
    cerr << "Error with listen" << endl;
    exit(-1);
  }
}

// Goes through accept socket logic and handles errors internally
int acceptSocket(int sock) {
  struct sockaddr_in clientAddr;
  socklen_t addrLen = sizeof(clientAddr);
  int clientSock = accept(sock, (struct sockaddr*) &clientAddr,
                          &addrLen);
  if (clientSock < 0) {
    cerr << "Error with accept process." << endl;
    exit(-1);
  }
  return clientSock;
}

void sendSock(int sock, char * msg) {
  int bytes = sizeof(msg);
  int sent = 0;
  while (bytes < sizeof(msg)) {
    sent = send(sock, msg, sizeof(msg), 0);
    if (sent < 0) {
      cerr << "Error with send process" << endl;
      exit(-1);
    }
    msg += sent;
    bytes -= sent;
  }
}

void recvSock(int sock, char * buf) {

}

