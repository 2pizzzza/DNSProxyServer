#ifndef LOG_H
#define LOG_H
#include <netinet/in.h>

void log_init(const char *log_file);
void log_request(struct sockaddr_in *client_addr, const char *domain, const char *response_type);
void log_close(void);

#endif