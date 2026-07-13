#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <stddef.h>
#endif

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PORT "8080"
#define DEFAULT_BUFLEN 2048

#define HTML_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
#define CSS_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nConnection: close\r\n\r\n"
#define JS_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/javascript\r\nConnection: close\r\n\r\n"

#define DEBUG 0

char *readFile(const char *filename);
char *buildResponse(const char *filename, const char *header);
const char *getContentType(const char *filename);
bool endsWith(const char *end, const char *filename);

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
            #if DEBUG
                printf("Bytes received: %d\n", iResult);
                printf("Received request:\n%.*s\n", iResult, recvbuf);
            #endif
                char method[8];
                char path[256];
                sscanf(recvbuf, "%7s %255s", method, path);

                if(strcmp(path, "/") == 0){
                    strcpy(path, "/index.html");
                }

                const char *header = getContentType(path);
                if(header == NULL){
                    printf("Error making header,\n");
                    shutdown(ClientSocket, SD_SEND);
                    closesocket(ClientSocket);
                    closesocket(ListenSocket);
                    WSACleanup();
                }

            char *response = buildResponse(path + 1, header); // + 1 because path is /file.ext, so path + 1 becomes file.ext
                                                              
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

char *readFile(const char *filename){
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
    char *file = readFile(filename);

    if(file == NULL){
        printf("Failed to open file: %s.\n", filename);
        return NULL;
    }

    size_t header_len = strlen(header);
    size_t file_len = strlen(file);
    char *response = malloc(header_len + file_len + 1);
    
    memcpy(response, header, header_len);
    memcpy(response + header_len, file, file_len);

    response[header_len + file_len] = '\0';

    free(file);

    return response;
}

bool endsWith(const char *end, const char *filename){
    if(!end || !filename){
        return false;
    }

    size_t filename_len = strlen(filename);
    size_t end_len = strlen(end);

    if(end_len >= filename_len){
        return false;
    }

    return strncmp(filename + filename_len - end_len, end, end_len) == 0;
}

const char *getContentType(const char *filename){
    if(endsWith(".html", filename)){
        return HTML_HEADER;
    }
    else if(endsWith(".js", filename)){
        return JS_HEADER;
    }
    else if(endsWith(".css", filename)){
        return CSS_HEADER;
    }
    else{
        return NULL;
    }
}
