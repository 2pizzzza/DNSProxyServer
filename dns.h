#ifndef DNS_H
#define DNS_H
#include <stdint.h>
#include <netinet/in.h>
#include "config.h"

#define DNS_PORT 53
#define MAX_PACKET_SIZE 512

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header_t;

typedef struct {
    char *qname;
    uint16_t qtype;
    uint16_t qclass;
} dns_question_t;

typedef struct {
    char *name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    char *rdata;
} dns_rr_t;

int dns_init_socket(int port);
int dns_receive_packet(int sock, char *buffer, int bufsize, struct sockaddr_in *client_addr, socklen_t *addr_len);
int dns_parse_packet(char *buffer, int len, dns_header_t *header, dns_question_t *question);
int dns_is_blacklisted(const char *qname, config_t *cfg);
void dns_generate_response(char *buffer, int *len, dns_header_t *header, dns_question_t *question, config_t *cfg);
int dns_forward_packet(char *buffer, int len, config_t *cfg, char *response, int *resp_len);
int dns_send_response(int sock, char *buffer, int len, struct sockaddr_in *client_addr, socklen_t addr_len);

#endif