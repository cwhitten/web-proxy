#ifndef CACHE
#define CACHE

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
	unordered_map<string, CacheEntry*> cache;
	unordered_map<string, CacheEntry*>::iterator it;

	public:
		~Cache() {
			for(it = cache.begin(); it != cache.end(); it++){
				delete it->second;
			}
		}
		void add(string key, CacheEntry * value) {
      if (cache.size() > 5) {
        lru();
      }
			cache[key] = value;
		}
		CacheEntry * get(string key) {
			if (cache.find(key) != cache.end()) {
        if (cache[key]->isFresh()) {
				  cache[key] -> updateAccessTime();
				  return cache[key];
        } else {
          delete cache[key];
          cache.erase(key);
        }
      }
			return NULL;
		}
		unsigned size() {
			return cache.size();
		}
		void lru(){
			CacheEntry* minEntry = cache.begin()->second;
			string minRequest = cache.begin()->first;
			for(it = cache.begin(); it != cache.end(); it++){
        if(it->second->getLastAccess() < minEntry->getLastAccess()){
					minRequest = it->first;
					minEntry = it->second;
				}
			}
			cache.erase(minRequest);
      delete minEntry;
		}
		// dump to file
		bool dumpToFile(char* fileName){
			ofstream file;
			file.open(fileName);
			file << cache.size() << '\n';
			for(it = cache.begin(); it != cache.end(); it++){
				file << it->first + '\n';
				file << it->second->getCharString() + '\n';
			}
			return true;
		}
		bool getFromFile(char* fileName){
			ifstream file;
			int size;
			string line;

			file >> size;
			file.open(fileName);
			for(int i = 0;i < size; i++){
				string currentRequest;
				// get the request string
				while(getline(file, line)){
					currentRequest+=line;
					if(currentRequest.substr(currentRequest.length()-8)
						== "\r\n\r\n"){
						break;
					}
				}
				// get the CacheEntry
				string header;
				string body;
				string entryTime;
				string lastAccessTime;
				// get header
				while(getline(file, line)){
					// inbetween headers and body \r\n\r\n
					header+=line;
					if(header.substr(header.length()-8) ==
						"\r\n\r\n"){
						break;
					}
				}
				// get body
				while(getline(file, line)){
					body+=line;
					if(body.substr(body.length()-8)
						=="\r\n\r\n"){
						break;
					}
				}
				// get time
				getline(file, entryTime);
				getline(file, lastAccessTime);
				cache[currentRequest] = NULL;
			}
			return true;
		}

};

#endif
