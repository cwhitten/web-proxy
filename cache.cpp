#include <tr1/unordered_map>
#include <iostream>
#include "request.cpp"

// using namespace std;
using namespace tr1;

class Cache{
	unordered_map<Request*, char*> cache;

};