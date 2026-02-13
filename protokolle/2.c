#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG_DIR "/var/log/maker/"

void get_logfile(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size,
             LOG_DIR "connection_state%Y.%m.%d_%H.log", t);
}

int ping_host(const char *host, int count) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "ping -q -c %d %s > /dev/null 2>&1", count, host);
    return system(cmd) == 0;
}

void test_host(const char *hostname, FILE *log) {
    char ip[128] = {0};
    char cmd[256];

    // DNS auflösen
    snprintf(cmd, sizeof(cmd),
             "dig +short %s | head -1", hostname);

    FILE *fp = popen(cmd, "r");
    if (fp && fgets(ip, sizeof(ip), fp)) {
        ip[strcspn(ip, "\n")] = 0;
    }
    if (fp) pclose(fp);

    // falls DNS fehlgeschlagen
    if (strlen(ip) == 0) {
        strncpy(ip, hostname, sizeof(ip) - 1);
    }

    if (ping_host(ip, 4)) {
        printf("%s (%s): ✓\n", hostname, ip);
        fprintf(log, "%s = True\n", ip);
    } else {
        printf("%s (%s): ✗\n", hostname, ip);
        fprintf(log, "%s = False\n", ip);
    }
}

int main() {
    char logfile[256];
    get_logfile(logfile, sizeof(logfile));

    FILE *log = fopen(logfile, "w");
    if (!log) {
        perror("Logdatei konnte nicht erstellt werden");
        return 1;
    }

    test_host("10.1.200.5", log);
    sleep(4);

    for (int i = 2; i <= 6; i++) {
        char host[64];
        snprintf(host, sizeof(host), "nasabsof%d", i);
        test_host(host, log);
        sleep(4);
    }

    fclose(log);
    return 0;
}
