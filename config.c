#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#include <yaml.h>

static resp_type_t parse_resp(const char *s, char *ip_out) {
    if (strcasecmp(s, "NXDOMAIN") == 0) return RESP_NXDOMAIN;
    if (strcasecmp(s, "REFUSED") == 0) return RESP_REFUSED;
    strncpy(ip_out, s, IP_STR_LEN-1);
    return RESP_IP;
}

int load_config(const char *path, config_t *cfg) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    yaml_parser_t parser;
    yaml_event_t event;
    if (!yaml_parser_initialize(&parser)) return -1;
    yaml_parser_set_input_file(&parser, f);
    char key[MAX_LINE] = {0}, value[MAX_LINE] = {0};
    int reading_list = 0;
    while (1) {
        if (!yaml_parser_parse(&parser, &event)) break;
        if (event.type == YAML_SCALAR_EVENT) {
            if (!key[0]) strcpy(key, (char*)event.data.scalar.value);
            else {
                strcpy(value, (char*)event.data.scalar.value);
                if (strcmp(key, "upstream") == 0) strncpy(cfg->upstream, value, IP_STR_LEN-1);
                else if (strcmp(key, "port") == 0) cfg->port = atoi(value);
                else if (strcmp(key, "blocked_response") == 0) cfg->block_resp = parse_resp(value, cfg->block_ip);
                else if (strcmp(key, "blacklist") == 0) reading_list = 1;
                else if (reading_list && cfg->blacklist_count < MAX_DOMAINS) {
                    cfg->blacklist[cfg->blacklist_count++] = strdup(value);
                }
                key[0]=value[0]=0;
            }
        } else if (event.type == YAML_SEQUENCE_END_EVENT) {
            reading_list = 0;
        }
        if (event.type == YAML_STREAM_END_EVENT) { yaml_event_delete(&event); break; }
        yaml_event_delete(&event);
    }
    yaml_parser_delete(&parser);
    fclose(f);
    return 0;
}