Authors: Josh Gummersall, Chris Whitten, Ken Ordona

Description:
  
  This program allows you to connect to a web host at a specific port and send a simple 
  HTTP request to that web host. For example, you can connect to www.google.com:80 and 
  send the following request:
      GET / HTTP/1.0

Use:

  -Run "make" to compile the source code

  -Type "./proxy <host_name> <port>"
    (Note 1: valid example of <host_name> is: www.google.com LEAVE OUT HTTP,
    HTTPS, FTP, etc. HTTP is assumed)

    (Note 2: if you leave out <host_name> AND <port> the program will connect to
    www.google.com on port 80 by default)

  -Input an HTTP request such as: "GET / HTTP/1.0" and press enter ONCE 
    (Note: the extra \r\n\r\n is appended to the request you type in before the request 
    is sent.)

  -The program will print out the response in the following format:
    <HEADERS>

    <HTML RESPONSE>

  -It is up to the user to input a valid host name, port, and request

  -Program assumes the server you connect to will close the socket when the
  response is complete

