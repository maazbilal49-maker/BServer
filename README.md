# BServer

A very minimal HTTP server built in C using Windows Winsock.

BServer is a small HTTP server created to explore how web servers work internally, including TCP sockets, HTTP responses, file handling, and client-server communication.

## Features

- TCP socket server using Winsock2
- HTTP/1.1 response handling
- Serves HTML files
- Serves CSS files
- Serves JavaScript files
- Custom 404 page support
- Manual HTTP response construction
- Simple request routing

## How it works

BServer follows a basic HTTP server workflow:

1. Creates a TCP socket
2. Binds the socket to port 8080
3. Listens for incoming connections
4. Accepts client requests
5. Reads the requested files
6. Builds an HTTP response
7. Sends the response back to the browser

Example request:
