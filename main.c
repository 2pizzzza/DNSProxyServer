#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include "config.h"
#include "dns.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config.yml>\n", argv[0]);
        return 1;
    }

    config_t cfg = {0};
    if (load_config(argv[1], &cfg) != 0) {
        fprintf(stderr, "Failed to load config\n");
        return 1;
    }

    int sock = dns_init_socket(cfg.port);
    if (sock < 0) {
        fprintf(stderr, "Failed to initialize socket\n");
        return 1;
    }

    printf("DNS proxy running on port %d, upstream %s\n", cfg.port, cfg.upstream);

    char buffer[MAX_PACKET_SIZE];
    char response[MAX_PACKET_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        int len = dns_receive_packet(sock, buffer, sizeof(buffer), &client_addr, &addr_len);
        if (len < 0) continue;

        dns_header_t header;
        dns_question_t question = {0};
        if (dns_parse_packet(buffer, len, &header, &question) < 0) {
            fprintf(stderr, "Invalid DNS packet\n");
            continue;
        }

        if (dns_is_blacklisted(question.qname, &cfg)) {
            dns_generate_response(buffer, &len, &header, &question, &cfg);
            dns_send_response(sock, buffer, len, &client_addr, addr_len);
        } else {
            int resp_len;
            if (dns_forward_packet(sock, buffer, len, &cfg, response, &resp_len) == 0) {
                dns_send_response(sock, response, resp_len, &client_addr, addr_len);
            }
        }
    }

    close(sock);
    for (int i = 0; i < cfg.blacklist_count; i++) {
        free(cfg.blacklist[i]);
    }
    return 0;
}