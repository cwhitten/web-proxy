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

  int parseContentLength(char * resp) {
    string response(resp);
    string length;
    int ind = 0;
    while (ind < strlen(response.c_str())) {
      if (response.substr(ind, 16) != "Content-Length: ") {
        ind++;
      } else {
        ind += 16;
        while (ind < strlen(response.c_str()) && response[ind] != '\n') {
          length += response[ind++];
        }
        return atoi(length.c_str());
      }
    }
    return 0;
  }

  int parseHeaderLength(char * resp) {
    string response(resp);
    int ind = 0;
    while (ind < strlen(response.c_str())) {
      if (response.substr(ind, 4) == "\r\n\r\n") {
        ind += 4;
        break;
      }
      ind++;
    }
    return ind;
  }

public:
  CacheEntry(char * response) {
    length = parseContentLength(response);
    if (length != 0) {
      length += parseHeaderLength(response);
    } else {
      length = strlen(response);
    }
    cachedResponse = new char[length];
    for (unsigned i = 0; i < length; i++) {
      cachedResponse[i] = response[i];
    }
    time(&entryTime);
    time(&lastAccessTime);
  }

  CacheEntry(char * response, std::string eTime, std::string aTime) {
    length = parseContentLength(response);
    if (length != 0) {
      length += parseHeaderLength(response);
    } else {
      length = strlen(response);
    }
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

  time_t getLastAccess(){
    return lastAccessTime;
  }
};

#endif
