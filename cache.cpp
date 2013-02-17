#include <tr1/unordered_map>
#include <iostream>
#include <pthread.h>
#include <fstream>
#include "request.cpp"
#include "cacheEntry.cpp"
#include "time.h"

// using namespace std;
using namespace tr1;

// check freshness, remove if not fresh
// dump cache to disk on close
class Cache{
	unordered_map<Request*, CacheEntry*> cache;
	unordered_map<Request*, CacheEntry*>::iterator it;

	public:
		bool checkFreshness(CacheEntry* entry, time_t currentTime){
			// TODO CHANGE FROM ARBITRARY CONDITION
			return currentTime - entry->getLastAccess() > 50;
		}
		void replace(Request* request, CacheEntry* entry){
			// unorder
			CacheEntry* minEntry = cache.begin()->second;
			Request* minRequest = cache.begin()->first;
			for(it = cache.begin(); it != cache.end(); it++){
				if(it->second->getLastAccess() < minEntry->getLastAccess()){
					minRequest = it->first;
					minEntry = it->second;
				}
			}
			cache.erase(minRequest);
		}
		// dump to file
		bool dumpToFile(char* fileName){
			ofstream file;
			file.open(fileName);
			for(it = cache.begin(); it != cache.end(); it++){
				file << it->first->hostName + "\n";
				file << it->first->pathName+ "\n";
				file << it->second->toString()+ "\n";
			}
			return true;
		}
		bool getFromFile(char* fileName){
			ifstream file;
			file.open(fileName);
		}

};