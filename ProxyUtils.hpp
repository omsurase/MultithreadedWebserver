#pragma once
#include <string>
#include "./CacheStrategy.hpp"
#include "./proxy_parse.h"

int createServerSocket(int port);
char *convertRequestToString(struct ParsedRequest *req);
int createServerSocket(const char *address, const char *port);
void writeToServerSocket(const char *buff, int sockfd, int length);
void writeToClientSocket(const char *buff, int sockfd, int length);
void writeToClient(int clientfd, int serverfd, CacheStrategy &cache, const char *url);