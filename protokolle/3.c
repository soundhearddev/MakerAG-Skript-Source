#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdbool.h>

// ==================== KONFIGURATION ====================
#define FRAME_DELAY 40000           
#define MESSAGE_START_FRAME 200     
#define MESSAGE_REVEAL_SPEED 40     
#define DROP_SPAWN_CHANCE 12        
#define MIN_DROP_SPEED 1            
#define MAX_DROP_SPEED 3            
#define MIN_TRAIL_LEN 5
#define MAX_TRAIL_LEN 12

// ==================== GEHEIME NACHRICHT ====================
const char *SECRET_MESSAGE = "SlVTVCBJTiBUSU1FICAgICAgIFBSIDM4";

// ==================== FARBEN ====================
#define COLOR_RESET         "\033[0m"
#define COLOR_DARK_GREEN    "\033[2;32m"    
#define COLOR_BRIGHT_GREEN  "\033[1;92m"    
#define COLOR_WHITE         "\033[1;92m"    

// ==================== STRUKTUREN ====================
typedef struct {
    int y;              
    int speed;          
    int trail_len;      
    bool active;        
    char *chars;
} Drop;

typedef struct {
    char ch;            
    int x;           
    int target_y;       
    int current_y;      
    bool revealed;
    bool falling;       
    bool arrived;
    int trail_len;
} MessageChar;

// ==================== GLOBALE VARIABLEN ====================
static int g_width = 0;
static int g_height = 0;
static Drop *g_drops = NULL;
static MessageChar *g_message = NULL;
static int g_message_len = 0;
static int g_message_index = 0;
static volatile bool g_running = true;

// ==================== SIGNAL HANDLER ====================
void handle_sigint(int sig) {
    (void)sig;
    g_running = false;
}

void cleanup(void) {
    printf("\033[2J\033[H\033[?25h");
    fflush(stdout);
    
    if (g_drops) {
        for (int i = 0; i < g_width; i++) {
            if (g_drops[i].chars) free(g_drops[i].chars);
        }
        free(g_drops);
    }
    if (g_message) free(g_message);
}

// ==================== TERMINAL ====================
void get_terminal_size(int *width, int *height) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        *width = 80;
        *height = 24;
    } else {
        *width = w.ws_col;
        *height = w.ws_row;
    }
}

void set_cursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

// ==================== ZUFALLSZEICHEN ====================
char get_random_char(void) {
    const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return chars[rand() % (sizeof(chars) - 1)];
}

// ==================== NACHRICHT INITIALISIEREN ====================
void init_message(void) {
    g_message_len = strlen(SECRET_MESSAGE);
    g_message = calloc(g_message_len, sizeof(MessageChar));
    
    // Nachricht horizontal zentriert
    int message_y = g_height / 2;
    int start_x = (g_width - (g_message_len * 2)) / 2;
    if (start_x < 2) start_x = 2;
    
    for (int i = 0; i < g_message_len; i++) {
        g_message[i].ch = SECRET_MESSAGE[i];
        g_message[i].x = start_x + (i * 2);  // 2 Zeichen Abstand
        g_message[i].target_y = message_y;
        g_message[i].current_y = 1;
        g_message[i].revealed = false;
        g_message[i].falling = false;
        g_message[i].arrived = false;
        g_message[i].trail_len = 6 + rand() % 6;
    }
    
    g_message_index = 0;
}

// ==================== DROP VERWALTUNG ====================
void init_drop(Drop *drop) {
    drop->y = -(rand() % 30);
    drop->speed = MIN_DROP_SPEED + rand() % (MAX_DROP_SPEED - MIN_DROP_SPEED + 1);
    drop->trail_len = MIN_TRAIL_LEN + rand() % (MAX_TRAIL_LEN - MIN_TRAIL_LEN + 1);
    drop->active = true;
    
    if (drop->chars) free(drop->chars);
    drop->chars = malloc(drop->trail_len);
    
    for (int i = 0; i < drop->trail_len; i++) {
        drop->chars[i] = get_random_char();
    }
}

void init_all_drops(void) {
    for (int x = 0; x < g_width; x++) {
        g_drops[x].chars = NULL;
        if (rand() % 100 < 25) {
            init_drop(&g_drops[x]);
        } else {
            g_drops[x].active = false;
            g_drops[x].y = -(rand() % 40);
        }
    }
}

void update_drop(Drop *drop) {
    if (!drop->active) {
        if (rand() % 100 < DROP_SPAWN_CHANCE) {
            init_drop(drop);
        }
        return;
    }

    drop->y += drop->speed;
    
    if (rand() % 8 == 0) {
        int idx = rand() % drop->trail_len;
        drop->chars[idx] = get_random_char();
    }
    
    if (drop->y > g_height + drop->trail_len + 5) {
        drop->active = false;
        drop->y = -(rand() % 40);
    }
}

void render_drop(Drop *drop, int x) {
    if (!drop->active) return;

    for (int i = 0; i < drop->trail_len; i++) {
        int y = drop->y - i;
        
        if (y < 1 || y > g_height) continue;

        float intensity = 1.0f - ((float)i / drop->trail_len);
        
        set_cursor(x + 1, y);
        
        if (i == 0 || intensity > 0.7f) {
            printf("%s%c%s", COLOR_BRIGHT_GREEN, drop->chars[i], COLOR_RESET);
        } else if (intensity > 0.4f) {
            printf("%s%c%s", COLOR_BRIGHT_GREEN, drop->chars[i], COLOR_RESET);
        } else {
            printf("%s%c%s", COLOR_DARK_GREEN, drop->chars[i], COLOR_RESET);
        }
    }
}

// ==================== NACHRICHT ====================
void update_message_char(MessageChar *msg) {
    if (!msg->revealed || msg->arrived) return;
    
    if (msg->falling) {
        msg->current_y++;
        
        if (msg->current_y >= msg->target_y) {
            msg->current_y = msg->target_y;
            msg->falling = false;
            msg->arrived = true;
        }
    }
}

void reveal_next_letter(int frame) {
    if (frame < MESSAGE_START_FRAME) return;
    if (g_message_index >= g_message_len) return;
    
    int frames_since_start = frame - MESSAGE_START_FRAME;
    
    // Alle MESSAGE_REVEAL_SPEED Frames einen neuen Buchstaben
    if (frames_since_start % MESSAGE_REVEAL_SPEED == 0 && frames_since_start > 0) {
        if (g_message_index < g_message_len) {
            g_message[g_message_index].revealed = true;
            g_message[g_message_index].falling = true;
            g_message_index++;
        }
    }
}

void render_message(void) {
    for (int i = 0; i < g_message_len; i++) {
        MessageChar *msg = &g_message[i];
        
        if (!msg->revealed || msg->ch == ' ') continue;
        
        update_message_char(msg);
        
        int x = msg->x;
        int y = msg->current_y;
        
        // Zeichne Trail nur wenn fallend
        if (msg->falling) {
            for (int t = 1; t <= msg->trail_len; t++) {
                int trail_y = y - t;
                if (trail_y >= 1 && trail_y <= g_height) {
                    set_cursor(x, trail_y);
                    
                    float intensity = 1.0f - ((float)t / msg->trail_len);
                    
                    if (intensity > 0.5f) {
                        printf("%s%c%s", COLOR_WHITE, msg->ch, COLOR_RESET);
                    } else {
                        printf("%s%c%s", COLOR_DARK_GREEN, get_random_char(), COLOR_RESET);
                    }
                }
            }
        }
        
        // Zeichne Hauptbuchstaben
        if (y >= 1 && y <= g_height) {
            set_cursor(x, y);
            printf("%s%c%s", COLOR_WHITE, msg->ch, COLOR_RESET);
        }
    }
}

// ==================== HAUPTPROGRAMM ====================
int main(void) {
    signal(SIGINT, handle_sigint);
    atexit(cleanup);

    srand(time(NULL));
    get_terminal_size(&g_width, &g_height);

    if (g_width < 40 || g_height < 15) {
        fprintf(stderr, "Terminal zu klein! Mindestens 40x15 erforderlich.\n");
        return EXIT_FAILURE;
    }

    g_drops = calloc(g_width, sizeof(Drop));
    if (!g_drops) {
        perror("Speicher-Allokation fehlgeschlagen");
        return EXIT_FAILURE;
    }

    init_all_drops();
    init_message();

    printf("\033[2J\033[?25l");
    fflush(stdout);

    int frame = 0;

    while (g_running) {
        frame++;

        // Screen komplett neu zeichnen (gegen Artefakte)
        printf("\033[2J\033[H");

        // Update Phase
        for (int x = 0; x < g_width; x++) {
            update_drop(&g_drops[x]);
        }
        
        // Render Phase
        for (int x = 0; x < g_width; x++) {
            render_drop(&g_drops[x], x);
        }
        
        reveal_next_letter(frame);
        render_message();

        fflush(stdout);
        usleep(FRAME_DELAY);

        if (frame % 100 == 0) {
            int new_w, new_h;
            get_terminal_size(&new_w, &new_h);
            if (new_w != g_width || new_h != g_height) {
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}