#ifndef CACHE_ENTRY_H
#define CACHE_ENTRY_H

#include <stdlib.h>
#include <iostream>
#include "string.h"
#include "time.h"

class CacheEntry {
private:
  std::string httpRequest;
  std::string httpResponseHeaders;
  std::string httpResponseBody;
  time_t entryTime;
  time_t lastAccessTime;

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

public:
  CacheEntry() {
    httpRequest = "GET / HTTP/1.0";
    time(&entryTime);
    time(&lastAccessTime);
  };

  CacheEntry(std::string req, std::string resHead, std::string resBody,
              std::string etime, std::string atime) {
    httpRequest = req;
    httpResponseHeaders = resHead;
    httpResponseBody = resBody;
    entryTime = stringToTime(etime);
    lastAccessTime = stringToTime(atime);
  }

  std::string toString() {
    return  httpResponseHeaders + "\r\n\r\n" +
            httpResponseBody + "\r\n\r\n" +
            timeToString(entryTime) + '\n' +
            timeToString(lastAccessTime) + '\n';
  }
  time_t getLastAccess(){
    return lastAccessTime;
  }
};

#endif
