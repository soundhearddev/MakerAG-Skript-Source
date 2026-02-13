#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>
#include <limits.h>
#include <termios.h>

/* ============================================================================
 * Enhanced Debian Maintenance Tool 
 * ============================================================================ */

/* Color Definitions */
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_RED       "\033[0;31m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_BLUE      "\033[0;34m"
#define COLOR_CYAN      "\033[0;36m"
#define COLOR_MAGENTA   "\033[0;35m"
#define COLOR_WHITE     "\033[1;37m"
#define COLOR_RESET     "\033[0m"
#define COLOR_BOLD      "\033[1m"
#define COLOR_INFO      COLOR_CYAN

/* Path Configuration */
#define SCRIPT_DIR      "/var/script"
#define PROTOCOL_DIR    "/var/script/protokolle"
#define LOG_FILE        "/var/log/maintenance_script.log"
#define MAX_PATH_LEN    4096
#define MAX_CMD_LEN     8192
#define MAX_INPUT_LEN   256

/* Global Variables */
static int show_timestamps = 1;

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/* Get current timestamp */
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* Log message to file */
void log_message(const char *level, const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) return;
    
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    fprintf(log, "[%s] [%s] %s\n", timestamp, level, message);
    fclose(log);
}

/* Print functions with logging */
void print_success(const char *message) {
    printf("%s ✓ %s %s\n", COLOR_GREEN, COLOR_RESET, message);
    log_message("SUCCESS", message);
}

void print_error(const char *message) {
    printf("%s ✗ %s %s\n", COLOR_RED, COLOR_RESET, message);
    log_message("ERROR", message);
}

void print_warning(const char *message) {
    printf("%s ⚠ %s %s\n", COLOR_YELLOW, COLOR_RESET, message);
    log_message("WARNING", message);
}

void print_info(const char *message) {
    printf(" ", COLOR_CYAN, COLOR_RESET, message);
}

void print_separator(void) {
    printf("%s────────────────────────────────────────────────────────────────────%s\n",
           COLOR_BLUE, COLOR_RESET);
}

/* Print header with system info */
void print_header(void) {
    system("clear");
    printf("%s", COLOR_CYAN);
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                          ║\n");
    printf("║   ███╗   ███╗ █████╗ ██╗███╗   ██╗████████╗███████╗███╗   ██╗ █████╗   ║\n");
    printf("║   ████╗ ████║██╔══██╗██║████╗  ██║╚══██╔══╝██╔════╝████╗  ██║██╔══██╗  ║\n");
    printf("║   ██╔████╔██║███████║██║██╔██╗ ██║   ██║   █████╗  ██╔██╗ ██║███████║  ║\n");
    printf("║   ██║╚██╔╝██║██╔══██║██║██║╚██╗██║   ██║   ██╔══╝  ██║╚██╗██║██╔══██║  ║\n");
    printf("║   ██║ ╚═╝ ██║██║  ██║██║██║ ╚████║   ██║   ███████╗██║ ╚████║██║  ██║  ║\n");
    printf("║   ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═══╝╚═╝  ╚═╝  ║\n");
    printf("║                                                                          ║\n");  
    printf("║                    System Wartungs-Tool                                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);
    
    if (show_timestamps) {
        char timestamp[64];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S", tm_info);
        
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        
        printf("%sSystemzeit: %s%s%s\n", COLOR_WHITE, COLOR_CYAN, timestamp, COLOR_RESET);
        printf("%sHostname:   %s%s%s\n", COLOR_WHITE, COLOR_CYAN, hostname, COLOR_RESET);
    }
    printf("\n");
}

/* Wait for keypress */
void press_any_key(void) {
    printf("\n%sDrücke eine beliebige Taste zum Fortfahren...%s\n", COLOR_YELLOW, COLOR_RESET);
    getchar();
    getchar();
}

/* Confirm action */
int confirm_action(const char *message) {
    printf("%s%s (j/N): %s", COLOR_YELLOW, message, COLOR_RESET);
    char response = getchar();
    getchar(); // consume newline
    printf("\n");
    return (response == 'j' || response == 'J' || response == 'y' || response == 'Y');
}

/* Execute command and display output */
int execute_command(const char *command) {
    int status = system(command);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* ============================================================================
 * Main Menu Functions
 * ============================================================================ */

void system_update(void) {
    print_header();
    printf("%s%s═══ System Update ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    print_separator();
    
    printf("\n");
    print_info("Update der Paketlisten...");
    
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "apt update 2>&1 | tee -a %s", LOG_FILE);
    
    if (execute_command(cmd) == 0) {
        print_success("Paketlisten aktualisiert");
    } else {
        print_error("Fehler beim Aktualisieren der Paketlisten");
        press_any_key();
        return;
    }
    
    printf("\n");
    print_info("Prüfe verfügbare Upgrades...");
    
    FILE *fp = popen("apt list --upgradable 2>/dev/null | grep -c 'upgradable'", "r");
    int upgradable = 0;
    if (fp) {
        fscanf(fp, "%d", &upgradable);
        pclose(fp);
    }
    
    if (upgradable > 0) {
        printf("%s%d Pakete können aktualisiert werden%s\n\n", COLOR_CYAN, upgradable, COLOR_RESET);
        
        if (confirm_action("Möchtest du die Upgrades installieren?")) {
            snprintf(cmd, sizeof(cmd), "apt upgrade -y 2>&1 | tee -a %s", LOG_FILE);
            execute_command(cmd);
            print_success("System-Upgrade abgeschlossen");
            
            printf("\n");
            if (confirm_action("Möchtest du auch ein dist-upgrade durchführen?")) {
                snprintf(cmd, sizeof(cmd), "apt dist-upgrade -y 2>&1 | tee -a %s", LOG_FILE);
                execute_command(cmd);
                print_success("Distribution-Upgrade abgeschlossen");
            }
            
            printf("\n");
            print_info("Aufräumen nicht benötigter Pakete...");
            snprintf(cmd, sizeof(cmd), "apt autoremove -y 2>&1 | tee -a %s", LOG_FILE);
            execute_command(cmd);
            snprintf(cmd, sizeof(cmd), "apt autoclean -y 2>&1 | tee -a %s", LOG_FILE);
            execute_command(cmd);
            print_success("Systembereinigung abgeschlossen");
        }
    } else {
        print_success("System ist bereits auf dem neuesten Stand");
    }
    
    press_any_key();
}

void view_logs(void) {
    print_header();
    printf("%s%s═══ System Logs ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    print_separator();
    
    printf("\n1) Syslog (letzte 30 Zeilen)\n");
    printf("2) Auth Log (letzte 30 Zeilen)\n");
    printf("3) Apache2 Error Log\n");
    printf("4) Apache2 Access Log\n");
    printf("5) Kernel Log\n");
    printf("6) Maintenance Script Log\n");
    printf("7) Live-Ansicht (tail -f)\n");
    printf("0) Zurück\n");
    printf("\nAuswahl: ");
    
    int choice;
    scanf("%d", &choice);
    
    printf("\n");
    print_separator();
    
    struct stat st;
    char cmd[MAX_CMD_LEN];
    
    switch (choice) {
        case 1:
            print_info("Syslog:");
            execute_command("tail -n 30 /var/log/syslog");
            break;
        case 2:
            print_info("Auth Log:");
            execute_command("tail -n 30 /var/log/auth.log");
            break;
        case 3:
            if (stat("/var/log/apache2/error.log", &st) == 0) {
                print_info("Apache2 Error Log:");
                execute_command("tail -n 30 /var/log/apache2/error.log");
            } else {
                print_error("Apache2 Error Log nicht gefunden");
            }
            break;
        case 4:
            if (stat("/var/log/apache2/access.log", &st) == 0) {
                print_info("Apache2 Access Log:");
                execute_command("tail -n 30 /var/log/apache2/access.log");
            } else {
                print_error("Apache2 Access Log nicht gefunden");
            }
            break;
        case 5:
            print_info("Kernel Log:");
            execute_command("dmesg | tail -n 30");
            break;
        case 6:
            if (stat(LOG_FILE, &st) == 0) {
                print_info("Maintenance Script Log:");
                snprintf(cmd, sizeof(cmd), "tail -n 30 %s", LOG_FILE);
                execute_command(cmd);
            } else {
                print_error("Log-Datei nicht gefunden");
            }
            break;
        case 7:
            print_info("Live-Ansicht (Strg+C zum Beenden)");
            printf("\n");
            execute_command("tail -f /var/log/syslog");
            break;
        case 0:
            return;
        default:
            print_error("Ungültige Auswahl");
            break;
    }
    
    press_any_key();
}

void service_management(void) {
    print_header();
    printf("%s%s═══ Service Management ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    print_separator();
    
    printf("\n1) Apache2 Status\n");
    printf("2) MySQL/MariaDB Status\n");
    printf("3) SSH Status\n");
    printf("4) Alle Services anzeigen\n");
    printf("0) Zurück\n");
    printf("\nAuswahl: ");
    
    int choice;
    scanf("%d", &choice);
    
    printf("\n");
    print_separator();
    
    switch (choice) {
        case 1:
            print_info("Apache2 Status:");
            execute_command("systemctl status apache2 --no-pager");
            break;
        case 2:
            print_info("MySQL/MariaDB Status:");
            if (execute_command("systemctl status mysql --no-pager 2>/dev/null") != 0) {
                if (execute_command("systemctl status mariadb --no-pager 2>/dev/null") != 0) {
                    print_error("MySQL/MariaDB nicht installiert");
                }
            }
            break;
        case 3:
            print_info("SSH Status:");
            execute_command("systemctl status ssh --no-pager");
            break;
        case 4:
            print_info("Aktive Services:");
            execute_command("systemctl list-units --type=service --state=running --no-pager | head -n 30");
            break;
        case 0:
            return;
        default:
            print_error("Ungültige Auswahl");
            break;
    }
    
    press_any_key();
}

void system_info(void) {
    print_header();
    printf("%s%s═══ System Information ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    print_separator();
    
    printf("\n%sSystem:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  OS:          %s", COLOR_CYAN);
    execute_command("lsb_release -d | cut -f2");
    printf("%s", COLOR_RESET);
    
    printf("  Kernel:      %s", COLOR_CYAN);
    execute_command("uname -r");
    printf("%s", COLOR_RESET);
    
    printf("  Architektur: %s", COLOR_CYAN);
    execute_command("uname -m");
    printf("%s\n", COLOR_RESET);
    
    printf("%sHardware:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  CPU:         %s", COLOR_CYAN);
    execute_command("lscpu | grep 'Model name' | cut -d: -f2 | xargs");
    printf("%s", COLOR_RESET);
    
    printf("  Cores:       %s", COLOR_CYAN);
    execute_command("nproc");
    printf("%s", COLOR_RESET);
    
    printf("  RAM Total:   %s", COLOR_CYAN);
    execute_command("free -h | awk '/^Mem:/ {print $2}'");
    printf("%s", COLOR_RESET);
    
    printf("  RAM Benutzt: %s", COLOR_CYAN);
    execute_command("free -h | awk '/^Mem:/ {print $3}'");
    printf("%s", COLOR_RESET);
    
    printf("  RAM Frei:    %s", COLOR_CYAN);
    execute_command("free -h | awk '/^Mem:/ {print $4}'");
    printf("%s\n", COLOR_RESET);
    
    printf("%sFestplatten:%s\n", COLOR_BOLD, COLOR_RESET);
    execute_command("df -h | grep -E '^/dev/'");
    
    printf("\n%sNetzwerk:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  IPs: %s", COLOR_CYAN);
    execute_command("hostname -I");
    printf("%s", COLOR_RESET);
    
    printf("\n%sLaufende Prozesse:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  Total: %s", COLOR_CYAN);
    execute_command("ps aux | wc -l");
    printf("%s", COLOR_RESET);
    
    press_any_key();
}

void disk_cleanup(void) {
    print_header();
    printf("%s%s═══ Festplatten-Bereinigung ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    print_separator();
    
    printf("\n");
    print_info("Analysiere Festplattenspeicher...");
    printf("\n%sAktuelle Belegung:%s\n", COLOR_BOLD, COLOR_RESET);
    execute_command("df -h | grep -E '^/dev/'");
    printf("\n");
    
    print_info("Bereinigungsoptionen:");
    printf("1) APT Cache leeren\n");
    printf("2) Alte Kernel entfernen\n");
    printf("3) Temporäre Dateien löschen\n");
    printf("4) Alte Logs rotieren\n");
    printf("5) Alle Bereinigungen durchführen\n");
    printf("6) aktuelle Speichernutzung anzeigen\n");
    printf("0) Zurück\n");
    printf("\nAuswahl: ");
    
    int choice;
    scanf("%d", &choice);
    
    printf("\n");
    print_separator();
    
    switch (choice) {
        case 1:
        case 5:
            print_info("Leere APT Cache...");
            execute_command("apt clean");
            execute_command("apt autoclean");
            print_success("APT Cache geleert");
            if (choice != 5) break;
            /* fall through */
        case 2:
            print_info("Entferne alte Kernel...");
            execute_command("apt autoremove --purge -y");
            print_success("Alte Kernel entfernt");
            if (choice != 5) break;
            /* fall through */
        case 3:
            print_info("Lösche temporäre Dateien...");
            execute_command("rm -rf /tmp/* /var/tmp/* 2>/dev/null");
            print_success("Temporäre Dateien gelöscht");
            if (choice != 5) break;
            /* fall through */
        case 4:
            print_info("Rotiere alte Logs...");
            execute_command("journalctl --vacuum-time=7d");
            print_success("Logs rotiert");
            break;
        case 6:
            execute_command("dysk");
        case 0:
            return;
        default:
            print_error("Ungültige Auswahl");
            break;
    }
    
    if (choice != 0) {
        printf("\n");
        print_info("Neue Belegung:");
        execute_command("df -h | grep -E '^/dev/'");
    }
    
    press_any_key();
}

void settings_menu(void) {
    print_header();
    printf("%s%s═══ Einstellungen ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    print_separator();
    
    printf("\n1) Zeitstempel anzeigen: ");
    if (show_timestamps) {
        printf("%sAN%s\n", COLOR_GREEN, COLOR_RESET);
    } else {
        printf("%sAUS%s\n", COLOR_RED, COLOR_RESET);
    }
    printf("2) Log-Datei anzeigen\n");
    printf("0) Zurück\n");
    printf("\nAuswahl: ");
    
    int choice;
    scanf("%d", &choice);
    
    struct stat st;
    char cmd[MAX_CMD_LEN];
    
    switch (choice) {
        case 1:
            show_timestamps = !show_timestamps;
            if (show_timestamps) {
                print_success("Zeitstempel eingeblendet");
            } else {
                print_success("Zeitstempel ausgeblendet");
            }
            sleep(1);
            settings_menu();
            break;
        case 2:
            if (stat(LOG_FILE, &st) == 0) {
                snprintf(cmd, sizeof(cmd), "less %s", LOG_FILE);
                execute_command(cmd);
            } else {
                print_error("Log-Datei nicht gefunden");
                press_any_key();
            }
            break;
        case 0:
            return;
        default:
            print_error("Ungültige Auswahl");
            sleep(1);
            settings_menu();
            break;
    }
}



/* Execute a single protocol with optional input piping */
int execute_protocol(const char *protocol_name, const char *pipe_input) {
    char protocol_path[MAX_PATH_LEN];
    snprintf(protocol_path, sizeof(protocol_path), "%s/%s", PROTOCOL_DIR, protocol_name);
    
    struct stat st;
    if (stat(protocol_path, &st) != 0) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Protokoll '%s' nicht gefunden", protocol_name);
        print_error(msg);
        return -1;
    }
    
    if (!(st.st_mode & S_IXUSR)) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Protokoll '%s' ist nicht ausführbar", protocol_name);
        print_error(msg);
        return -1;
    }
    
    char msg[512];
    snprintf(msg, sizeof(msg), "Führe Protokoll '%s' aus...", protocol_name);
    print_info(msg);
    
    int status;
    
    if (pipe_input != NULL && strlen(pipe_input) > 0) {
        /* Execute with piped input */
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "echo '%s' | %s", pipe_input, protocol_path);
        status = execute_command(cmd);
    } else {
        /* Execute normally */
        status = execute_command(protocol_path);
    }
    
    return status;
}

/* Parse and execute protocol chain (supports piping) */
int execute_protocol_chain(const char *input) {
    char input_copy[MAX_CMD_LEN];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0';
    
    /* Check if piping is used */
    char *pipe_pos = strstr(input_copy, "|");
    
    if (pipe_pos == NULL) {
        /* No piping - check for multiple protocols separated by space or comma */
        char *token;
        char *saveptr;
        int total = 0;
        int success = 0;
        
        token = strtok_r(input_copy, " ,", &saveptr);
        while (token != NULL) {
            /* Trim whitespace */
            while (*token == ' ') token++;
            if (strlen(token) == 0) {
                token = strtok_r(NULL, " ,", &saveptr);
                continue;
            }
            
            total++;
            printf("\n");
            int status = execute_protocol(token, NULL);
            
            if (status == 0) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Protokoll '%s' erfolgreich", token);
                print_success(msg);
                success++;
            } else {
                char msg[512];
                snprintf(msg, sizeof(msg), "Protokoll '%s' fehlgeschlagen (Code: %d)", token, status);
                print_error(msg);
            }
            
            token = strtok_r(NULL, " ,", &saveptr);
        }
        
        if (total > 1) {
            printf("\n");
            print_separator();
            printf("%sZusammenfassung: %d/%d Protokolle erfolgreich%s\n", 
                   COLOR_CYAN, success, total, COLOR_RESET);
        }
        
        return (success == total) ? 0 : 1;
    } else {
        /* Piping detected */
        char *protocols[32];
        int count = 0;
        
        /* Use a separate buffer for tokenization to avoid issues */
        char pipe_copy[MAX_CMD_LEN];
        strncpy(pipe_copy, input_copy, sizeof(pipe_copy) - 1);
        pipe_copy[sizeof(pipe_copy) - 1] = '\0';
        
        char *saveptr = NULL;
        char *token = strtok_r(pipe_copy, "|", &saveptr);
        
        while (token != NULL && count < 32) {
            /* Trim whitespace */
            while (*token == ' ' || *token == '\t') token++;
            
            /* Remove trailing whitespace */
            char *end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) {
                *end = '\0';
                end--;
            }
            
            if (strlen(token) > 0) {
                /* Allocate memory and copy the token */
                protocols[count] = malloc(strlen(token) + 1);
                if (protocols[count] == NULL) {
                    print_error("Speicherfehler");
                    /* Free previously allocated memory */
                    for (int j = 0; j < count; j++) {
                        free(protocols[j]);
                    }
                    return -1;
                }
                strcpy(protocols[count], token);
                count++;
            }
            
            token = strtok_r(NULL, "|", &saveptr);
        }
        
        if (count == 0) {
            print_error("Keine Protokolle angegeben");
            return -1;
        }
        
        printf("\n%sPipe-Chain: %d Protokoll(e)%s\n", COLOR_YELLOW, count, COLOR_RESET);
        
        /* Validate all protocols exist before execution */
        for (int i = 0; i < count; i++) {
            char protocol_path[MAX_PATH_LEN];
            snprintf(protocol_path, sizeof(protocol_path), "%s/%s", PROTOCOL_DIR, protocols[i]);
            
            struct stat st;
            if (stat(protocol_path, &st) != 0) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Protokoll '%s' nicht gefunden", protocols[i]);
                print_error(msg);
                /* Free allocated memory */
                for (int j = 0; j < count; j++) {
                    free(protocols[j]);
                }
                return -1;
            }
            
            if (!(st.st_mode & S_IXUSR)) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Protokoll '%s' nicht ausführbar", protocols[i]);
                print_error(msg);
                /* Free allocated memory */
                for (int j = 0; j < count; j++) {
                    free(protocols[j]);
                }
                return -1;
            }
        }
        
        /* Build and execute pipe command */
        char cmd[MAX_CMD_LEN * 2];
        int offset = 0;
        
        for (int i = 0; i < count; i++) {
            if (i > 0) {
                offset += snprintf(cmd + offset, sizeof(cmd) - offset, " | ");
            }
            offset += snprintf(cmd + offset, sizeof(cmd) - offset, "%s/%s", PROTOCOL_DIR, protocols[i]);
        }
        
        printf("\n%sAusführe: %s%s\n\n", COLOR_CYAN, cmd, COLOR_RESET);
        
        int status = execute_command(cmd);
        
        /* Free allocated memory */
        for (int i = 0; i < count; i++) {
            free(protocols[i]);
        }
        
        if (status == 0) {
            printf("\n");
            print_success("Pipe-Chain erfolgreich abgeschlossen");
        } else {
            printf("\n");
            print_error("Pipe-Chain mit Fehler beendet");
        }
        
        return status;
    }
}

void protokolle(void) {
    /* Clear input buffer from previous menu */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    while (1) {
        print_header();
        printf("%s%s═══ Protokolle ausführen ═══%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
        print_separator();
        
        printf("\n");
        printf("Protokollnummer eingeben (q zum Beenden):\n");
        printf("\n");
        print_separator();
        printf("\nEingabe: ");
        
        char input[MAX_CMD_LEN];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            continue;
        }
        
        /* Remove newline */
        input[strcspn(input, "\n")] = 0;
        
        /* Trim leading/trailing whitespace */
        char *trimmed = input;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        
                /* Check for quit */
        if (strcmp(trimmed, "q") == 0 || strcmp(trimmed, "Q") == 0) {
            print_info("Protokoll-Modus verlassen");
            sleep(1);
            return;
        }
        
        
        
        /* Check for empty input */
        if (strlen(trimmed) == 0) {
            print_warning("Keine Eingabe");
            sleep(1);
            continue;
        }
        
        /* Execute protocol(s) */
        printf("\n");
        print_separator();
        int result = execute_protocol_chain(trimmed);
        
        if (result == 0) {
            log_message("INFO", "Protokoll-Ausführung erfolgreich");
        } else {
            log_message("WARNING", "Protokoll-Ausführung mit Fehler");
        }
        
        press_any_key();
    }
}

/* ============================================================================
 * Main Menu
 * ============================================================================ */

void main_menu(void) {
    while (1) {
        print_header();
        print_separator();
        printf("%s%sHauptmenü:%s\n\n", COLOR_BOLD, COLOR_WHITE, COLOR_RESET);
        printf("  %s[1]%s System Update durchführen\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[2]%s System-Logs anzeigen\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[3]%s Service Management\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[4]%s System-Informationen\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[5]%s Festplatten-Bereinigung\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[6]%s Protokolle ausführen\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[7]%s Einstellungen\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[8]%s Pull Latest update\n", COLOR_CYAN, COLOR_RESET);
        printf("  %s[0]%s Beenden\n\n", COLOR_RED, COLOR_RESET);
        print_separator();
        printf("Auswahl: ");
        
        int option;
        if (scanf("%d", &option) != 1) {
            /* Clear invalid input */
            while (getchar() != '\n');
            print_error("Ungültige Eingabe");
            sleep(1);
            continue;
        }
        
        switch (option) {
            case 1:
                system_update();
                break;
            case 2:
                view_logs();
                break;
            case 3:
                service_management();
                break;
            case 4:
                system_info();
                break;
            case 5:
                disk_cleanup();
                break;
            case 6:
                protokolle();
                break;
            case 7:
                settings_menu();
                break;
            case 0:
                system("clear");
                printf("%s", COLOR_GREEN);
                printf("╔════════════════════════════════════════╗\n");
                printf("║  Vielen Dank für die Nutzung!         ║\n");
                printf("║  Auf Wiedersehen!                     ║\n");
                printf("╚════════════════════════════════════════╝\n");
                printf("%s\n", COLOR_RESET);
                log_message("INFO", "Script beendet");
                exit(0);
            default:
                print_error("Ungültige Option");
                sleep(1);
                break;
        }
    }
}

/* ============================================================================
 * Main Function
 * ============================================================================ */

int main(int argc, char *argv[]) {
    /* Check for root privileges */
    if (geteuid() != 0) {
        fprintf(stderr, "%sFehler: Dieses Programm muss mit sudo ausgeführt werden!%s\n", 
                COLOR_RED, COLOR_RESET);
        fprintf(stderr, "Verwende: sudo %s\n", argv[0]);
        return 1;
    }
    
    /* Initialize log file */
    FILE *log = fopen(LOG_FILE, "a");
    if (log != NULL) {
        fclose(log);
        chmod(LOG_FILE, 0644);
    }
    
    /* Start script */
    log_message("INFO", "Maintenance Script gestartet (C Version)");
    
    /* Run main menu */
    main_menu();
    
    return 0;
}
