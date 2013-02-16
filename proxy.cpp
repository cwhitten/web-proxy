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

// Program constants
const int MAX_THREADS = 50;
const int MAX_PENDING = 5;
const bool LOG_TO_FILE = false;
const char * LOG_FILE_NAME = "proxy.log";
const int EXIT_SIGNAL = 2;
const int OK_CODE = 1;
const int BAD_CODE = -1;
const char * SERV_PORT = "10200";

// Global data structures
queue<string> REQUEST_QUEUE;

// Synchronization locks
sem_t LOGGING_LOCK;
sem_t REQUEST_QUEUE_LOCK;
sem_t ACTIVE_SOCKETS_LOCK;
pthread_cond_t CONSUME_COND = PTHREAD_COND_INITIALIZER;

struct threadInfo {
  unsigned long num;
};

// Function prototypes

// Loop MAX_THREADS times and create a new pthread
// to consume from request queue
void initializeThreadPool();

// Initialize the request queue that the threads will
// read from. This may be simple (std::queue)
void initializeRequestQueue();

// Add a request to the request std::queue
// Grab mutex lock, add request to std::queue
// and then unlock
void addRequest(string request);

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
  // Initialize locking mechanisms
  sem_init(&LOGGING_LOCK, 0, 1);
  sem_init(&REQUEST_QUEUE_LOCK, 0, 1);
  sem_init(&ACTIVE_SOCKETS_LOCK, 0, 1);

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
  log("Starting proxy server...");
  while (true) {
    clientSock = acceptSocket(sock);
    log("Accepted connection");
    // wait for incoming request
    // if a request comes in, parse it and add to request queue
    close(sock);
    break;
  }

  log("Closing socket.");
  close(sock);
  return 0;
}

void initializeThreadPool() {
  for(int i = 0; i < MAX_THREADS; i++) {
    // create thread and assign thread routine for each thread
    // using pthread_create()
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
  threadInfo * t = (threadInfo *) info;
  return NULL;
}

void addRequest(string request) {
  sem_wait(&REQUEST_QUEUE_LOCK);
  REQUEST_QUEUE.push(request);
  sem_post(&REQUEST_QUEUE_LOCK);
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
}
