#include "cacheEntry.cpp"
#include "string.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <istream>
#include "network.cpp"
using namespace std;

// Program constants
const int NUM_ARGS = 3;
const int HOST_INDEX = 1;
const int PORT_INDEX = 2;
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
vector<CacheEntry *> buildCache(); 
void dumpCache(vector<CacheEntry> v);

int main(int argc, char * argv[]) {

  return 0;
}

// Check for valid command line arguments
bool validCmdArgs(int argc, char* argv[]) {
  return argc == NUM_ARGS;
}

vector<CacheEntry *> buildCache() {
  vector<CacheEntry *> retCache;
  CacheEntry * c = NULL;
  ifstream inFile;
  inFile.open(CACHE_FILE);
  string request, headers, body, entry, accessed;
  string line;

  while (!inFile.eof()) {
    getline(inFile, request);
    getline(inFile, headers);
    getline(inFile, body);
    getline(inFile, entry);
    getline(inFile, accessed);
    c = new CacheEntry(request, headers, body, entry, accessed);
    retCache.push_back(c);

    // Garbage data
    getline(inFile, request);
    getline(inFile, request);
  }

  inFile.close();
  inFile.clear();
  return retCache;
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

