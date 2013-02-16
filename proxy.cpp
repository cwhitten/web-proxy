// Standard includes
#include <stdlib.h>
#include <signal.h>
#include <iostream>
using namespace std;

// File includes
#include "cacheEntry.cpp"
#include "network.cpp"

// Program constants
const int MAX_THREADS = 50;
const int EXIT_SIGNAL = 2;
const int OK_CODE = 1;
const int BAD_CODE = -1;

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
void * consumeRequest(void * ptr);

// Generic function to log messages about the proxy. This
// will either write to a file or the stdout, haven't
// decided yet
void log(string message);

// Exit handler code that will be bound to the Ctrl+C
// signal. Code should handle shutting threads and dumping
// the cache to disk
void exitHandler(int signal);

int main(int argc, char * argv[]) {
  // Bind Ctrl+C Signal to Exit Handler
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = exitHandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);

  // Initialize socket
  // Initialize mutex
  // Intialize request queue
  // Initialize thread pool
  
  while (true) {
    // wait for incoming request
    // if a request comes in, parse it and add to request queue
  }

  return 0;
}

void log(string message) {
  cout << "LOG: " << message << endl;
}

void exitHandler(int signal) {
  if (signal == EXIT_SIGNAL) {
    log("Shutting down proxy.");
    exit(OK_CODE);
  }
}

