Authors: Josh Gummersall, Chris Whitten, Ken Ordona

Description:
This proxy server sits on a machine and listens to requests. It then forwards
those requests and responds with the response from the server the packets were
forwarder to.

Cache Details:
-Caching replacement is handled by the LRU algorithm
-The TTL is set to 1000 seconds
-Cache is built on the unordered_map data structure for constant lookup time
-Cache is not persistent (ran out of time)

Known Limitations:
We run out of sockets after 100 requests because of a bug with how they are closed in pthreads
Caching is not persistent (ran out of time)

Use:
-Run "make" to compile the source code
-Run ./proxy to run the proxy server
-Proxy listens on whatever port SERV_PORT is set to in proxy.cpp

