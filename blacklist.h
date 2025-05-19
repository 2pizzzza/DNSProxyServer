#ifndef BLACKLIST_H
#define BLACKLIST_H
#include "config.h"

int dns_is_blacklisted(const char *qname, config_t *cfg);
char *qname_to_domain(const char *qname, char *domain, int max_len);

#endif