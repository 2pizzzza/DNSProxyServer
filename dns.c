#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dns.h"
#include "blacklist.h"

int dns_init_socket(int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }
    return sock;
}

int dns_receive_packet(int sock, char *buffer, int bufsize, struct sockaddr_in *client_addr, socklen_t *addr_len) {
    int len = recvfrom(sock, buffer, bufsize, 0, (struct sockaddr*)client_addr, addr_len);
    if (len < 0) {
        perror("recvfrom");
        return -1;
    }
    return len;
}

int dns_parse_packet(char *buffer, int len, dns_header_t *header, dns_question_t *question) {
    if (len < 12) return -1;

    header->id = ntohs(*(uint16_t*)buffer);
    header->flags = ntohs(*(uint16_t*)(buffer + 2));
    header->qdcount = ntohs(*(uint16_t*)(buffer + 4));
    header->ancount = ntohs(*(uint16_t*)(buffer + 6));
    header->nscount = ntohs(*(uint16_t*)(buffer + 8));
    header->arcount = ntohs(*(uint16_t*)(buffer + 10));

    if (header->qdcount != 1) return -1;

    int pos = 12;
    question->qname = buffer + pos;

    while (pos < len && buffer[pos] != 0) pos += buffer[pos] + 1;
    pos++;

    if (pos + 4 > len) return -1;  // Ensure space for QTYPE and QCLASS
    question->qtype = ntohs(*(uint16_t*)(buffer + pos));
    question->qclass = ntohs(*(uint16_t*)(buffer + pos + 2));
    return pos + 4;
}

void dns_generate_response(char *buffer, int *len, dns_header_t *header, dns_question_t *question, config_t *cfg) {
    int pos = 12 + strlen(question->qname) + 1 + 4;  // Header + QNAME + QTYPE + QCLASS
    header->flags = htons(0x8180);
    header->ancount = 0;

    switch (cfg->block_resp) {
        case RESP_NXDOMAIN:
            header->flags = htons(0x8183);  // NXDOMAIN
            break;
        case RESP_REFUSED:
            header->flags = htons(0x8185);  // REFUSED
            break;
        case RESP_IP:
            header->flags = htons(0x8180);  // Success
            header->ancount = htons(1);
            // Add answer section
            buffer[pos++] = 0xc0;  // Name pointer to QNAME
            buffer[pos++] = 0x0c;
            *(uint16_t*)(buffer + pos) = htons(1);  // TYPE A
            pos += 2;
            *(uint16_t*)(buffer + pos) = htons(1);  // CLASS IN
            pos += 2;
            *(uint32_t*)(buffer + pos) = htonl(3600);  // TTL
            pos += 4;
            *(uint16_t*)(buffer + pos) = htons(4);  // RDLENGTH
            pos += 2;
            inet_pton(AF_INET, cfg->block_ip, buffer + pos);  // RDATA (IP)
            pos += 4;
            break;
    }

    *(uint16_t*)buffer = htons(header->id);
    *(uint16_t*)(buffer + 2) = header->flags;
    *(uint16_t*)(buffer + 4) = htons(header->qdcount);
    *(uint16_t*)(buffer + 6) = header->ancount;
    *(uint16_t*)(buffer + 8) = htons(0);  // NSCOUNT
    *(uint16_t*)(buffer + 10) = htons(0);  // ARCOUNT
    *len = pos;
}

int dns_forward_packet(char *buffer, int len, config_t *cfg, char *response, int *resp_len) {
    int upstream_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (upstream_sock < 0) {
        perror("upstream socket");
        return -1;
    }

    struct sockaddr_in upstream_addr = {0};
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_port = htons(DNS_PORT);
    inet_pton(AF_INET, cfg->upstream, &upstream_addr.sin_addr);

    if (sendto(upstream_sock, buffer, len, 0, (struct sockaddr*)&upstream_addr, sizeof(upstream_addr)) < 0) {
        perror("sendto upstream");
        close(upstream_sock);
        return -1;
    }

    *resp_len = recvfrom(upstream_sock, response, MAX_PACKET_SIZE, 0, NULL, NULL);
    if (*resp_len < 0) {
        perror("recvfrom upstream");
        close(upstream_sock);
        return -1;
    }

    close(upstream_sock);
    return 0;
}

int dns_send_response(int sock, char *buffer, int len, struct sockaddr_in *client_addr, socklen_t addr_len) {
    if (sendto(sock, buffer, len, 0, (struct sockaddr*)client_addr, addr_len) < 0) {
        perror("sendto client");
        return -1;
    }
    return 0;
}