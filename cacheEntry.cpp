#ifndef CACHE_ENTRY_H
#define CACHE_ENTRY_H

#include <stdlib.h>
#include <iostream>
#include "string.h"
#include "time.h"

using namespace std;

class CacheEntry {
private:
  char * cachedResponse;
  time_t entryTime;
  time_t lastAccessTime;
  int length;

  // Convert time_t object to std::string
  std::string timeToString(time_t t) {
    char buf[80];
    struct tm * ptr;
    ptr = localtime(&t);
    strftime (buf, 80, "%H:%M:%S, %m/%d/%Y", ptr);
    std::string s;
    for (unsigned i = 0; i < strlen(buf); i++) {
      s += buf[i];
    }
    return s;
  };

  // Convert std::string object to time_t
  time_t stringToTime(std::string ts) {

    // Go through ugly code to grab pieces of std::string ts
    int hour, minute, second, year, month, day;
    std::string tmp;
    const char * t = ts.c_str();
    unsigned i = 0;
    while (i < strlen(t) && t[i] != ':') {
      tmp += t[i++];
    }
    hour = atoi(tmp.c_str());
    tmp = "";
    i++;

    while (i < strlen(t) && t[i] != ':') {
      tmp += t[i++];
    }
    minute = atoi(tmp.c_str());
    tmp = "";
    i++;

    while (i < strlen(t) && t[i] != ',') {
      tmp += t[i++];
    }
    second = atoi(tmp.c_str());
    tmp = "";
    i++;

    while (i < strlen(t) && t[i] != '/') {
      tmp += t[i++];
    }
    month = atoi(tmp.c_str());
    tmp = "";
    i++;

    while (i < strlen(t) && t[i] != '/') {
      tmp += t[i++];
    }
    day = atoi(tmp.c_str());
    tmp = "";
    i++;

    while (i < strlen(t) && t[i] != '/') {
      tmp += t[i++];
    }
    year = atoi(tmp.c_str());

    // Get current local time, update fields, and return new time
    time_t rawTime;
    struct tm * timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    timeInfo->tm_year = year - 1900;
    timeInfo->tm_mon = month - 1;
    timeInfo->tm_mday = day;
    timeInfo->tm_sec = second;
    timeInfo->tm_min = minute;
    timeInfo->tm_hour = hour;
    return mktime(timeInfo);
  };

  int parseLength(char * resp) {
    string response(resp);
    string length;
    unsigned limit = response.find("\r\n\r\n");
    unsigned found = response.find("Content-Length: ");
    if (found == std::string::npos || found >= limit) {
      return (int) response.length();
    }
    found += 16;
    while (response[found] != '\n' && found < response.length())
      length += response[found++];
    found = response.find("\r\n\r\n");
    return found + atoi(length.c_str()); 
  }

  string parseContentType(char * resp) {
    string response(resp);
    string content;
    unsigned limit = response.find("\r\n\r\n");
    unsigned found = response.find("Content-Type: ");
    if (found == std::string::npos || found >= limit)
      return "";
    found += 14;
    while (response[found] != '\n')
      content += response[found++];
    return content;
  }

public:
  CacheEntry(char * response) {
    length = parseLength(response);
    cachedResponse = new char[length];
    for (unsigned i = 0; i < length; i++) {
      cachedResponse[i] = response[i];
    }
    time(&entryTime);
    time(&lastAccessTime);
  }

  CacheEntry(char * response, std::string eTime, std::string aTime) {
    length = parseLength(response);
    cachedResponse = new char[length + 1];
    for (unsigned i = 0; i < length; i++) {
      cachedResponse[i] = response[i];
    }
    entryTime = stringToTime(eTime);
    lastAccessTime = stringToTime(aTime);
  }

  ~CacheEntry() {
    delete [] cachedResponse;
  }

  void updateAccessTime() {
    time(&lastAccessTime);
  }

  char * getCharString() {
    return cachedResponse;
  }

  int getLength() {
    return length;
  }

  bool isFresh() {
    int entrySeconds = entryTime;
    int nowSeconds = time(NULL);
    return nowSeconds - entrySeconds < 10000;
  }

  bool isCacheable() {
    string contentType = parseContentType(cachedResponse);
    return contentType.find("text") == std::string::npos;
  }

  time_t getLastAccess(){
    return lastAccessTime;
  }
};

#endif
