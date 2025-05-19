#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include "config.h"

static resp_type_t parse_resp(const char *s, char *ip_out) {
    if (strcasecmp(s, "NXDOMAIN") == 0) return RESP_NXDOMAIN;
    if (strcasecmp(s, "REFUSED") == 0) return RESP_REFUSED;
    strncpy(ip_out, s, IP_STR_LEN-1);
    ip_out[IP_STR_LEN-1] = '\0';
    return RESP_IP;
}

int load_config(const char *path, config_t *cfg) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    yaml_parser_t parser;
    yaml_event_t event;
    if (!yaml_parser_initialize(&parser)) {
        fclose(f);
        return -1;
    }
    yaml_parser_set_input_file(&parser, f);

    char key[MAX_LINE] = {0};
    int in_blacklist = 0;
    cfg->blacklist_count = 0;
    memset(cfg->blacklist, 0, sizeof(cfg->blacklist));

    while (1) {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "YAML parse error: %s\n", parser.problem);
            yaml_parser_delete(&parser);
            fclose(f);
            return -1;
        }

        switch (event.type) {
            case YAML_SCALAR_EVENT: {
                const char *value = (char*)event.data.scalar.value;
                if (!in_blacklist) {
                    if (!key[0]) {
                        strcpy(key, value);
                    } else {
                        if (strcmp(key, "upstream") == 0) {
                            strncpy(cfg->upstream, value, IP_STR_LEN-1);
                            cfg->upstream[IP_STR_LEN-1] = '\0';
                        } else if (strcmp(key, "port") == 0) {
                            cfg->port = atoi(value);
                        } else if (strcmp(key, "blocked_response") == 0) {
                            cfg->block_resp = parse_resp(value, cfg->block_ip);
                        }
                        key[0] = 0;
                    }
                } else {
                    if (cfg->blacklist_count < MAX_DOMAINS) {
                        cfg->blacklist[cfg->blacklist_count] = strdup(value);
                        printf("Loaded blacklist[%d]: %s\n", cfg->blacklist_count, cfg->blacklist[cfg->blacklist_count]);
                        cfg->blacklist_count++;
                    }
                }
                break;
            }
            case YAML_SEQUENCE_START_EVENT:
                if (strcmp(key, "blacklist") == 0) {
                    in_blacklist = 1;
                }
                break;
            case YAML_SEQUENCE_END_EVENT:
                in_blacklist = 0;
                break;
            case YAML_STREAM_END_EVENT:
                yaml_event_delete(&event);
                goto done;
            default:
                break;
        }
        yaml_event_delete(&event);
    }

done:
    yaml_parser_delete(&parser);
    fclose(f);
    if (cfg->blacklist_count == 0) {
        fprintf(stderr, "Warning: No blacklist entries loaded\n");
    }
    return 0;
}