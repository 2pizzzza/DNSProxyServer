#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "config.h"


int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Usage: %s config.yml\n", argv[0]); return 1; }
    config_t cfg = {0};
    if (load_config(argv[1], &cfg) != 0) { perror("load_config"); return 1; }

    return 0;
}
