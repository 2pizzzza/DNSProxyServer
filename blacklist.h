#ifndef BLACKLIST_H
#define BLACKLIST_H
#include "config.h"

int dns_is_blacklisted(const char *qname, config_t *cfg);

#endif