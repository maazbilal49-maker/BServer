#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT "8080"
#define DEFAULT_BUFLEN 2048

#define HTML_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
#define CSS_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nConnection: close\r\n\r\n"
#define JS_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/javascript\r\nConnection: close\r\n\r\n"

char *readHTMLFile(const char *filename);
char *buildResponse(const char *filename, const char *header);

int main(void){
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(iResult != 0){
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, PORT, &hints, &result);
    if(iResult != 0){
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(ListenSocket == INVALID_SOCKET){
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
    if(iResult == SOCKET_ERROR){
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if(iResult == SOCKET_ERROR){
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server successfully started at port %s...", PORT);

    while(1){
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if(ClientSocket == INVALID_SOCKET){
            printf("accpet failed with error: %d", WSAGetLastError());
            continue;
        }

        memset(recvbuf, 0, recvbuflen);
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if(iResult > 0){
            printf("Received request:\n%.*s\n", iResult, recvbuf);

            char *response = NULL;

            if(strstr(recvbuf, "GET /style.css")){
                response = buildResponse("style.css", CSS_HEADER);
            }
            else if(strstr(recvbuf, "GET /index.js")){
                response = buildResponse("index.js", JS_HEADER);
            }
            
            else{
                response = buildResponse("index.html", HTML_HEADER);
            }

            if(response == NULL){
                response = buildResponse("404.html", HTML_HEADER);
            }

            send(ClientSocket, response, (int)strlen(response), 0);

            free(response);
        }
        shutdown(ClientSocket, SD_SEND);
        closesocket(ClientSocket);
    }
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}

char *readHTMLFile(const char *filename){
    FILE *f = fopen(filename, "r");
    if(!f){
        printf("Failed to open html file.");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    rewind(f);

    char *html = malloc(fileSize + 1);
    if(html == NULL){
        printf("Failed to allocate memory for response.\n");
        fclose(f);
        return NULL;
    }

    size_t bytesRead = fread(html, sizeof(char), fileSize, f);

    html[bytesRead] = '\0';

    fclose(f);
    return html;
}

char *buildResponse(const char *filename, const char *header){
    char *file = readHTMLFile(filename);

    if(file == NULL){
        printf("Failed to open file: %s.\n", filename);
        return NULL;
    }

    int header_len = strlen(header);
    int file_len = strlen(file);
    char *response = malloc(header_len + file_len + 1);
    
    memcpy(response, header, header_len);
    memcpy(response + header_len, file, file_len);

    response[header_len + file_len] = '\0';

    free(file);

    return response;
}