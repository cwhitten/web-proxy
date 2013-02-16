// Standard includes
#include <stdlib.h>
#include <iostream>
using namespace std;

// File includes
#include "cacheEntry.cpp"
#include "network.cpp"

// Program constants
const int MAX_THREADS = 50;

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

int main(int argc, char * argv[]) {
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
  cout << "LOG:" << message << endl;
}
