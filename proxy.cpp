#include "cacheEntry.cpp"
#include "string.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <istream>
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
const int BUF_SIZE = 80000;
const bool PRODUCTION = false;
const int CACHE_SIZE = 1000;
const int CACHE_TTL = 5 * 60;
const char * CACHE_FILE = "cacheDump.out";

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
char * recvSock(int sock);
vector<CacheEntry> buildCache(); 
void dumpCache(vector<CacheEntry> v);

int main(int argc, char * argv[]) {

  vector<CacheEntry> cache;
  CacheEntry c1, c2, c3;
  cache.push_back(c1);
  cache.push_back(c2);
  cache.push_back(c3);
  dumpCache(cache);

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
  cout << "Ctrl+C to escape." << endl << endl;

  // Grab request
  string request;
  getline(cin, request);
  cout << endl;
  request = request + '\r' + '\n' + '\r' + '\n';
  char * req = (char*) request.c_str();

  // Send request and receive response
  sendSock(sock, req);
  char * out = recvSock(sock);
  cout << out << endl;
  free(out);

  // Close socket
  close(sock);

  return 0;
}

// Check for valid command line arguments
bool validCmdArgs(int argc, char* argv[]) {
  return argc == NUM_ARGS;
}

// Return host from command line args
char * getHost(char * args[]) {
  return args[HOST_INDEX];
}

// Return port from command line args
char * getPort(char * args[]) {
  return args[PORT_INDEX];
}

// Converts host name into server IP address
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

// Send msg to sock ensuring to resend if the entire message is not sent
void sendSock(int sock, char * msg) {
  int bytesToSend = strlen(msg), bytesSent = 0, totalBytesSent = 0;
  while (bytesToSend > 0) {
    bytesSent = send(sock, msg, bytesToSend, 0);
    if (bytesSent < 0) {
      cerr << "Error sending request." << endl;
      exit(-1);
    }
    totalBytesSent += bytesSent;
    bytesToSend -= bytesSent;
    msg += bytesSent;
  }
}

// Receive response from socket. This method loops and receives response
// until the server explicity closes the socket (recv returns zero)
char * recvSock(int sock) {
  char * buffer = NULL, * newbuffer;
  int pos = 0, len = 0, found = 0;
  int i, n;
  bool first = false;

  while (true) {
    if (pos == len) {
      len = len ? len << 1 : 4;
      if (len >= BUF_SIZE) {
        len = BUF_SIZE;
      }
      newbuffer = (char *) realloc(buffer, len);
      if (!newbuffer) {
        free(buffer);
        cerr << "Out of memory." << endl;
        return NULL;
      }
      buffer = newbuffer;
    }
    n = recv(sock, buffer + pos, len - pos, 0);
    if (n < 0) {
      cerr << "Error receiving from socket." << endl;
      free(buffer);
      return NULL;
    }
    if (n == 0) {
      return buffer;
    }
    pos += n;
  }
}

vector<CacheEntry> buildCache() {
  ifstream inFile;
  inFile.open(CACHE_FILE);
  string request, headers, body, entry, accessed;
  string line;

  while (inFile.good()) {
    getline(inFile, line);
    cout << line << endl;
  }

  inFile.close();
  inFile.clear();
};

void dumpCache(vector<CacheEntry> v) {
  ofstream outFile;
  outFile.open(CACHE_FILE);
  if (!outFile.is_open()) {
    cout << "Error opening file." << endl;
  }
  for (unsigned i = 0; i < v.size(); i++) {
    outFile << v[i].toString();
  }
  outFile.close();
  outFile.clear();
}

