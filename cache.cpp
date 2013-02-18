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
		bool checkFreshness(CacheEntry* entry, time_t currentTime){
			// TODO CHANGE FROM ARBITRARY CONDITION
			return currentTime - entry->getLastAccess() > 50;
		}
		void replace(Request* request, CacheEntry* entry){
			// unorder
			CacheEntry* minEntry = cache.begin()->second;
			string minRequest = cache.begin()->first;
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
			file << cache.size() << endl;
			for(it = cache.begin(); it != cache.end(); it++){
				file << it->first + "\n";
				file << it->second->toString();
			}
			return true;
		}
		bool getFromFile(char* fileName){
			ifstream file;
			file.open(fileName);
			int size;
			file >> size;
			for(int i = 0;i < size; i++){
				CacheEntry* currEntry;
				string hostName;
				string pathName;
				getline(file, hostName);
				getline(file, pathName);
			}

		}

};

#endif
