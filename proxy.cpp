// Standard includes
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <list>
#include <sstream>

#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

using namespace std;

// File includes
#include "cacheEntry.cpp"
#include "network.cpp"
#include "request.cpp"
#include "cache.cpp"

// Program constants
const int MAX_THREADS = 50;
const int MAX_PENDING = 5;
const bool LOG_TO_FILE = true;
const char * LOG_FILE_NAME = "proxy.log";
const int EXIT_SIGNAL = 2;
const int OK_CODE = 1;
const int BAD_CODE = -1;
const char * SERV_PORT = "10200";
const char * HTTP_PORT = "80";

// Global data structures
queue<Request *> REQUEST_QUEUE;
vector<int> SOCKET_VECTOR;
Cache HTTP_CACHE;

// Synchronization locks
sem_t LOGGING_LOCK;
pthread_mutex_t SOCKET_VECTOR_LOCK;
pthread_mutex_t REQUEST_QUEUE_LOCK;
pthread_mutex_t HTTP_CACHE_LOCK;
pthread_cond_t CONSUME_COND = PTHREAD_COND_INITIALIZER;

struct threadInfo {
  unsigned int num;
};

// Function prototypes

// Loop MAX_THREADS times and create a new pthread
// to consume from request queue
void initializeThreadPool();

// Initialize the request queue that the threads will
// read from. This may be simple (std::queue)
void initializeRequestQueue();

// Add a request to the request std::queue
// Grab lock, add request to std::queue and then unlock
void addRequest(Request * request);

// Pop a request off of the request queue and return it
// This is thread-safe as it is synchronized
Request * removeRequest();

// Clear all the remaining requests from the request queue
// and make sure to dellocate dynamic memory. This is not
// thread synchronized because it is called when exiting
// the program regardless of thread state.
void clearRequestQueue();

// Function that each thread will constantly be executing
// Thread should get mutex lock, process a request (remove
// from the queue) and then unlock the queue
void * consumeRequest(void * info);

string buildRequest(string host, string path);

// Function that will add an open socket to a global vector
// so that we can close open sockets on error or when program
// exits. This function is thread safe
void addSocket(int sock);

// Function that will close an open socket and remove it from
// global vector. This function is thread safe.
void closeSocket(int sock);

// Function to close all remaining open sockets. This will be
// executed when the program returns, is closed, or closes on
// error. This method is not thread synchronized because it is
// called on closing the program regardless of the state of the
// threads.
void closeOpenSockets();

void addToCache(string key, CacheEntry * value);
CacheEntry * checkCache(string key);

// Generic function to log messages about the proxy. This
// will either write to a file or the stdout, haven't
// decided yet
void log(string message, sem_t lock = LOGGING_LOCK);

// Function to return the current time as a string. This
// is used in the logging function
string getTime();

// Exit handler code that will be bound to the Ctrl+C
// signal. Code should handle shutting threads and dumping
// the cache to disk
void exitHandler(int signal);

// Exit handler code that will be bound to return event
// This should basically just log that the proxy is exitting
void returnHandler();

int main(int argc, char * argv[]) {
  // Initialize locking mechanisms
  sem_init(&LOGGING_LOCK, 0, 1);
  pthread_mutex_init(&REQUEST_QUEUE_LOCK, NULL);
  pthread_mutex_init(&SOCKET_VECTOR_LOCK, NULL);
  pthread_mutex_init(&HTTP_CACHE_LOCK, NULL);

  // Blank log so we know when we started
  log("");

  // Bind Ctrl+C Signal to exitHandler()
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = exitHandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);

  // Bind returnHandler() to return/exit event
  atexit(returnHandler);

  // TODO:
  // BUILD CACHE FROM DISK

  // Initialize socket
  int sock = getSocket();
  bindSocket(sock, (char *) SERV_PORT);
  addSocket(sock);
  listenSocket(sock, MAX_PENDING);

  // Initialize thread pool
  initializeThreadPool();

  int clientSock;
  char * request;
  log("Starting proxy server...");

  while (true) {
    log("Listening for a connection.");
    clientSock = acceptSocket(sock);
    addSocket(clientSock);
    request = recvRequest(clientSock);
    string req(request);
    cout << req << endl;
    if (strlen(req.c_str()) > 0) {
      log("Received non-empty request.");
      log(req);
      delete [] request;
      addRequest(new Request(req, clientSock));
    } else {
      log("Ignoring empty request.");
    }
  }

  log("Closing proxy socket.");
  closeSocket(sock);
  log("");
  return 0;
}

void initializeThreadPool() {
  for(int i = 0; i < MAX_THREADS; i++) {
    pthread_t tid;
    threadInfo curr;
    curr.num = i;

    int rc = pthread_create(&tid, NULL, consumeRequest,
      (void *)(&curr));
    if (rc) {
      log("ERROR in ThreadPool initialization");
      exit(BAD_CODE);
    }
    pthread_detach(tid);
  }
}

void * consumeRequest(void * info) {
  int sock;
  CacheEntry * entry;
  bool foundInCache = false;
  while (true) {
    pthread_cond_wait(&CONSUME_COND, &REQUEST_QUEUE_LOCK);
    if (!REQUEST_QUEUE.empty()) {
      // Grab a request
      Request * r = REQUEST_QUEUE.front();
      if (r == NULL) {
        return NULL;
      }
      REQUEST_QUEUE.pop();

      // Log request information
      log("Hostname: " + r->hostName);
      log("Path: " + r->pathName);

      // Build request
      string request = buildRequest(r->hostName, r->pathName);
      entry = checkCache(request);
      if (entry != NULL) {
        log("Cache hit.");

        // Send cached response
        log("Sending response to browser.");
        sendSock(r->getSock(), entry->toCharString());
      } else {
        log("Cache miss.");

        // Get IP address and connect socket
        char ip[20];
        hostnameToIp((char *) r->hostName.c_str(), ip);
        sock = getSocket();
        connectSocket(sock, ip, (char *) HTTP_PORT);
        addSocket(sock);

        // Make request of server and get response
        log("Making request " + request);
        sendSock(sock, (char *) request.c_str());
        log("Receiving response.");
        char * out = recvSock(sock);

        // Cache response (will be freed by Cache destructor)
        log("Caching HTTP response.");
        string resp(out);
        entry = new CacheEntry(request, resp);
        addToCache(request, entry);

        // Send response to browser
        log("Sending response to browser.");
        sendSock(r->getSock(), out);
        log("Freeing out variable");

        // Free out dynamic memory and close socket
        free(out);
        closeSocket(sock);
      }

      closeSocket(r->getSock());
      log("");
      delete r;
    }
    pthread_cond_signal(&CONSUME_COND);
  }
  return NULL;
}

string buildRequest(string host, string path) {
  return  "GET " + path + " HTTP/1.0\n" +
          "Host: " + host + "\n" +
          "User-Agent: TEST" + "\r\n\r\n";
}

void addRequest(Request * request) {
  pthread_mutex_lock(&REQUEST_QUEUE_LOCK);
  REQUEST_QUEUE.push(request);
  pthread_cond_signal(&CONSUME_COND);
  pthread_cond_wait(&CONSUME_COND, &REQUEST_QUEUE_LOCK);
  pthread_mutex_unlock(&REQUEST_QUEUE_LOCK);
}

Request * removeRequest() {
  pthread_mutex_lock(&REQUEST_QUEUE_LOCK);
  Request * r = REQUEST_QUEUE.front();
  REQUEST_QUEUE.pop();
  pthread_mutex_unlock(&REQUEST_QUEUE_LOCK);
  return r;
}

void clearRequestQueue() {
  log("Clearing request queue.");
  Request * r;
  while (!REQUEST_QUEUE.empty()) {
    r = REQUEST_QUEUE.front();
    REQUEST_QUEUE.pop();
    delete r;
  }
}

void addSocket(int sock) {
  pthread_mutex_lock(&SOCKET_VECTOR_LOCK);
  SOCKET_VECTOR.push_back(sock);
  pthread_mutex_unlock(&SOCKET_VECTOR_LOCK);
}

void closeSocket(int sock) {
  pthread_mutex_lock(&SOCKET_VECTOR_LOCK);
  for (unsigned i = 0; i < SOCKET_VECTOR.size(); i++) {
    if (SOCKET_VECTOR[i] == sock) {
      close(sock);
      SOCKET_VECTOR.erase(SOCKET_VECTOR.begin() + i);
    }
  }
  pthread_mutex_unlock(&SOCKET_VECTOR_LOCK);
}

void closeOpenSockets() {
  log("Closing all open sockets.");
  for (unsigned i = 0; i < SOCKET_VECTOR.size(); i++) {
    close(SOCKET_VECTOR[i]);
  }
  SOCKET_VECTOR.clear();
}

void addToCache(string key, CacheEntry * value) {
  pthread_mutex_lock(&HTTP_CACHE_LOCK);
  HTTP_CACHE.add(key, value);
  pthread_mutex_unlock(&HTTP_CACHE_LOCK);
}

CacheEntry * checkCache(string key) {
  pthread_mutex_lock(&HTTP_CACHE_LOCK);
  CacheEntry * c;
  c = HTTP_CACHE.get(key);
  pthread_mutex_unlock(&HTTP_CACHE_LOCK);
  return c;
}

void log(string message, sem_t lock) {
  string msg = getTime() + " LOG: " + message + "\n";
  sem_wait(&lock);
  if (LOG_TO_FILE) {
    ofstream out;
    out.open(LOG_FILE_NAME, ios::app);
    if (out.is_open()) {
      out << msg;
      out.close();
    }
  } else {
    cout << msg;
  }
  sem_post(&lock);
}

string getTime() {
  time_t t = time(0);
  ostringstream convert;
  struct tm * now = localtime(&t);
  int year = (now->tm_year + 1900);
  int month = (now->tm_mon + 1);
  int day = (now->tm_mday);
  int hour = now->tm_hour;
  int minute = now->tm_min;
  int second = now->tm_sec;
  convert << "[" << month << "/" << day << "/" << year;
  convert << "-" << hour << ":" << minute << ":" << second << "]";
  return convert.str();
}

void exitHandler(int signal) {
  if (signal == EXIT_SIGNAL) {
    log("Shutting down proxy server.");
    exit(OK_CODE);
  }
}

void returnHandler() {
  log("Proxy server has called exit()");
  clearRequestQueue();
  closeOpenSockets();
  // TODO:
  // DUMP CACHE TO DISK
  log("Exiting.");
}
