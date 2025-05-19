#include <string.h>
#include "blacklist.h"

static char *qname_to_domain(const char *qname, char *domain, int max_len) {
    int pos = 0, out_pos = 0;
    while (qname[pos] != 0 && out_pos < max_len - 1) {
        int len = (unsigned char)qname[pos++];
        for (int i = 0; i < len && out_pos < max_len - 1; i++) {
            domain[out_pos++] = qname[pos++];
        }
        if (qname[pos] != 0) domain[out_pos++] = '.';
    }
    domain[out_pos] = '\0';
    return domain;
}

int dns_is_blacklisted(const char *qname, config_t *cfg) {
    char domain[256];
    qname_to_domain(qname, domain, sizeof(domain));

    for (int i = 0; i < cfg->blacklist_count; i++) {
        if (strcasecmp(domain, cfg->blacklist[i]) == 0) {
            return 1;
        }
    }
    return 0;
}