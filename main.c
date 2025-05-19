#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blacklist.h"
#include "config.h"
#include "dns.h"
#include "log.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config.yml>\n", argv[0]);
        return 1;
    }

    log_init("dns_proxy.log");

    config_t cfg = {0};
    if (load_config(argv[1], &cfg) != 0) {
        fprintf(stderr, "Failed to load config\n");
        log_close();
        return 1;
    }

    int sock = dns_init_socket(cfg.port);
    if (sock < 0) {
        fprintf(stderr, "Failed to initialize socket\n");
        log_close();
        return 1;
    }

    printf("DNS proxy running on port %d, upstream %s\n", cfg.port, cfg.upstream);

    char buffer[MAX_PACKET_SIZE];
    char response[MAX_PACKET_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char domain[256];

    while (1) {
        int len = dns_receive_packet(sock, buffer, sizeof(buffer), &client_addr, &addr_len);
        if (len < 0) continue;

        dns_header_t header;
        dns_question_t question = {0};
        if (dns_parse_packet(buffer, len, &header, &question) < 0) {
            fprintf(stderr, "Invalid DNS packet\n");
            continue;
        }

        qname_to_domain(question.qname, domain, sizeof(domain));

        if (dns_is_blacklisted(question.qname, &cfg)) {
            char resp_str[IP_STR_LEN + 4];
            switch (cfg.block_resp) {
                case RESP_NXDOMAIN: strcpy(resp_str, "NXDOMAIN"); break;
                case RESP_REFUSED: strcpy(resp_str, "REFUSED"); break;
                case RESP_IP: snprintf(resp_str, sizeof(resp_str), "IP:%.60s", cfg.block_ip); break;
                default: strcpy(resp_str, "Unknown"); break;
            }
            log_request(&client_addr, domain, resp_str);

            dns_generate_response(buffer, &len, &header, &question, &cfg);
            dns_send_response(sock, buffer, len, &client_addr, addr_len);
        } else {
            log_request(&client_addr, domain, "Forwarded to upstream");

            int resp_len;
            if (dns_forward_packet(buffer, len, &cfg, response, &resp_len) == 0) {
                dns_send_response(sock, response, resp_len, &client_addr, addr_len);
            }
        }
    }

    close(sock);
    for (int i = 0; i < cfg.blacklist_count; i++) {
        free(cfg.blacklist[i]);
    }
    log_close();
    return 0;
}