#ifndef _NETWORK_CODE
#define _NETWORK_CODE

// Networking includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

const int BUF_SIZE = 80000;

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
      buffer[pos] = '\0';
      return buffer;
    }
    pos += n;
  }
}

// Receive an HTTP request from sock. This method will receive
// 100 bytes while looking for a \n delimeter. Once the delimeter
// is found it will return a char * string up to the delimiter
char * recvRequest(int sock) {
  char * buff = new char[1000], * received = NULL;
  int n = 0, size = 0;
  while (true) {
    n = recv(sock, buff + size, 1, 0);
    if (n < 0) {
      cerr << "Error receiving from socket." << endl;
      delete [] buff;
      exit(-1);
    }
    if (buff[size - 1] == '\n') {
      cout << "Found delim" << endl;
      buff[size - 1] == '\0';
      received = new char[size];
      strcpy(received, buff);
      delete [] buff;
      return received;
    }
    size += n;
    if (size >= 1000) {
      cerr << "Improper request." << endl;
      delete [] buff;
      return NULL;
    }
  }
}

#endif
