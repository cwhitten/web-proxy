// Standard includes
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <list>

#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

using namespace std;

// File includes
#include "cacheEntry.cpp"
#include "network.cpp"
#include "request.cpp"

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
const char * DELIM = "\n";

// Global data structures
queue<Request *> REQUEST_QUEUE;

// Synchronization locks
sem_t LOGGING_LOCK;
pthread_mutex_t REQUEST_QUEUE_LOCK;
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
// and make sure to dellocate dynamic memory
void clearRequestQueue();

// Function that each thread will constantly be executing
// Thread should get mutex lock, process a request (remove
// from the queue) and then unlock the queue
void * consumeRequest(void * info);

// Generic function to log messages about the proxy. This
// will either write to a file or the stdout, haven't
// decided yet
void log(string message, sem_t lock = LOGGING_LOCK);

// Exit handler code that will be bound to the Ctrl+C
// signal. Code should handle shutting threads and dumping
// the cache to disk
void exitHandler(int signal);

// Exit handler code that will be bound to return event
// This should basically just log that the proxy is exitting
void returnHandler();

int main(int argc, char * argv[]) {
  log("");
  // Initialize locking mechanisms
  sem_init(&LOGGING_LOCK, 0, 1);
  pthread_mutex_init(&REQUEST_QUEUE_LOCK, NULL);

  // Bind Ctrl+C Signal to exitHandler()
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = exitHandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);

  // Bind returnHandler() to return/exit event
  atexit(returnHandler);

  // Initialize socket
  log("Initializing socket.");
  int sock = getSocket();
  bindSocket(sock, (char *) SERV_PORT);
  log("Setting socket to listen.");
  listenSocket(sock, MAX_PENDING);

  // Initialize thread pool
  initializeThreadPool();

  int clientSock;
  char * request;
  log("Starting proxy server...");

  while (true) {
    log("Listening for a connection.");
    clientSock = acceptSocket(sock);
    log("Accepted connection.");
    request = recvRequest(clientSock);
    log("Received request.");
    string req(request);
    delete [] request;
    addRequest(new Request(req, clientSock));
    log("Closing client socket.");
    close(clientSock);
  }

  log("Closing socket.");
  close(sock);
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
  while (true) {
    pthread_cond_wait(&CONSUME_COND, &REQUEST_QUEUE_LOCK);
    if (!REQUEST_QUEUE.empty()) {
      log("Consuming request.");
      Request * r = REQUEST_QUEUE.front();
      REQUEST_QUEUE.pop();
      log("Hostname: " + r->hostName);
      log("Path: " + r->pathName);

      char ip[20];
      hostnameToIp((char *) r->hostName.c_str(), ip);
      string IP(ip);
      log("Ip Address: " + IP);

      int sock = getSocket();
      connectSocket(sock, ip, (char *) HTTP_PORT);
      log("Connected to server.");
      string request = "GET " + r->pathName + " HTTP/1.0";
      request += "\r\n\r\n";
      log("Making request " + request);
      sendSock(sock, (char *) request.c_str());
      log("Receiving response.");
      char * out = recvSock(sock);
      log("Sending response to browser.");
      sendSock(r->socket, out);
      free(out);
      log("Closing server socket.");
      close(sock);

      delete r;
    }
    pthread_cond_signal(&CONSUME_COND);
  }
  return NULL;
}

void addRequest(Request * request) {
  pthread_mutex_lock(&REQUEST_QUEUE_LOCK);
  log("Adding request to queue.");
  REQUEST_QUEUE.push(request);
  pthread_cond_signal(&CONSUME_COND);
  pthread_cond_wait(&CONSUME_COND, &REQUEST_QUEUE_LOCK);
  pthread_mutex_unlock(&REQUEST_QUEUE_LOCK);
}

Request * removeRequest() {
  pthread_mutex_lock(&REQUEST_QUEUE_LOCK);
  log("Removing request from queue.");
  Request * r = REQUEST_QUEUE.front();
  REQUEST_QUEUE.pop();
  pthread_mutex_unlock(&REQUEST_QUEUE_LOCK);
  return r;
}

void clearRequestQueue() {
  pthread_mutex_lock(&REQUEST_QUEUE_LOCK);
  log("Clearing request queue.");
  Request * r;
  while (!REQUEST_QUEUE.empty()) {
    r = REQUEST_QUEUE.front();
    REQUEST_QUEUE.pop();
    delete r;
  }
  pthread_mutex_unlock(&REQUEST_QUEUE_LOCK);
}

void log(string message, sem_t lock) {
  string msg = "LOG: " + message + "\n";
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

void exitHandler(int signal) {
  if (signal == EXIT_SIGNAL) {
    log("Shutting down proxy server.");
    exit(OK_CODE);
  }
}

void returnHandler() {
  log("Proxy server has called exit()");
  clearRequestQueue();
}
