#ifndef RESPONSE
#define RESPONSE

#include <vector>
#include <tr1/unordered_map>
using namespace std::tr1;

class Response {
  private:
    unordered_map<string, string> headers;

    void parseHeaders(string s) {
      vector<string> lines;
      string tmp;
      int index = 0;
      while (index < strlen(s.c_str())) {
        if (s[index] == '\n') {
          index++;
          lines.push_back(tmp);
          tmp = "";
        } else {
          tmp += s[index++];
        }
      }
      string key, value;
      bool found = false;
      for (unsigned i = 0; i < lines.size(); i++) {
        key = value = "";
        for (int j = 0; j < strlen(lines[i].c_str()); j++) {
          if (lines[i][j] == ':') {
            found = true;
          }
          if (!found)
            key += lines[i][j];
          else
            value += lines[i][j];
        }
        headers[key] = value;
      }
    }

  public:
    Response() {

    }

    Response(string resHeaders) {
      parseHeaders(resHeaders);
    }

    int getContentLength() {
      if (headers.find("content-length") != headers.end()) {
        string length = headers["content-length"];
        return atoi(length.c_str());
      } else {
        return 0;
      }
    }

    ~Response() {

    }
};

#endif

