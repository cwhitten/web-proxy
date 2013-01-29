#include <iostream>
using namespace std;

// Program constants
const int NUM_ARGS = 3;
const int HOST_INDEX = 1;
const int PORT_INDEX = 2;
const bool PRODUCTION = false;

// Function prototypes
bool validCmdArgs(int argc, char* argv[]);
char * getHost(char * args[]);
char * getPort(char * args[]);

int main(int argc, char * argv[])
{
  // Get user name and host name
  char * host, * port;
  bool args = validCmdArgs(argc, argv);
  if (!args) {
    cerr << "Invalid command line args specified." << endl;
    cerr << "Usage: \"./proxy <hostname> <port>\"" << endl;
    return -1;
  } else {
    host = getHost(argv);
    port = getPort(argv);
  }

  cout << "Requesting " << host << " at port " << port << endl;

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
