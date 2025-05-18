#ifndef CONFIG_H
#define CONFIG_H
#define MAX_LINE 256
#define MAX_DOMAINS 1024
#define IP_STR_LEN 64

typedef enum { RESP_NXDOMAIN, RESP_REFUSED, RESP_IP } resp_type_t;

typedef struct {
    char upstream[IP_STR_LEN];
    int port;
    resp_type_t block_resp;
    char block_ip[IP_STR_LEN];
    int blacklist_count;
    char *blacklist[MAX_DOMAINS];
} config_t;

int load_config(const char *path, config_t *cfg);
#endif