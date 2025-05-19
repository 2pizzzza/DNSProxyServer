#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "log.h"

static FILE *log_fp = NULL;
static int log_initialized = 0;

void log_init(const char *log_file) {
    if (log_file) {
        log_fp = fopen(log_file, "a");
        if (!log_fp) {
            perror("Failed to open log file");
            return;
        }
        log_initialized = 1;
    }
}

void log_request(struct sockaddr_in *client_addr, const char *domain, const char *response_type) {
    char timestamp[32];
    char client_ip[INET_ADDRSTRLEN];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);

    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, sizeof(client_ip));

    char message[512];
    snprintf(message, sizeof(message), "[%s] Client: %s, Domain: %s, Response: %s\n",
             timestamp, client_ip, domain, response_type);

    printf("%s", message);

    if (log_initialized && log_fp) {
        fputs(message, log_fp);
        fflush(log_fp);
    }
}

void log_close(void) {
    if (log_initialized && log_fp) {
        fclose(log_fp);
        log_fp = NULL;
        log_initialized = 0;
    }
}