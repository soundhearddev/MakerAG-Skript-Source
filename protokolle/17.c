#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <math.h>

void signal_handler(int sig) {
    // Ignoriere alle Signale
}

void clear_screen(void) {
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
}

void set_cursor(int row, int col) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
    write(STDOUT_FILENO, buf, len);
}

void hide_cursor(void) {
    write(STDOUT_FILENO, "\033[?25l", 6);
}

void show_cursor(void) {
    write(STDOUT_FILENO, "\033[?25h", 6);
}

void get_terminal_size(int *rows, int *cols) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        *rows = w.ws_row;
        *cols = w.ws_col;
    } else {
        *rows = 24;
        *cols = 80;
    }
}

const char* char_to_morse(char c) {
    c = (c >= 'a' && c <= 'z') ? (c - 32) : c; // Uppercase

    switch(c) {
        case 'A': return ".-";
        case 'B': return "-...";
        case 'C': return "-.-.";
        case 'D': return "-..";
        case 'E': return ".";
        case 'F': return "..-.";
        case 'G': return "--.";
        case 'H': return "....";
        case 'I': return "..";
        case 'J': return ".---";
        case 'K': return "-.-";
        case 'L': return ".-..";
        case 'M': return "--";
        case 'N': return "-.";
        case 'O': return "---";
        case 'P': return ".--.";
        case 'Q': return "--.-";
        case 'R': return ".-.";
        case 'S': return "...";
        case 'T': return "-";
        case 'U': return "..-";
        case 'V': return "...-";
        case 'W': return ".--";
        case 'X': return "-..-";
        case 'Y': return "-.--";
        case 'Z': return "--..";
        case '0': return "-----";
        case '1': return ".----";
        case '2': return "..---";
        case '3': return "...--";
        case '4': return "....-";
        case '5': return ".....";
        case '6': return "-....";
        case '7': return "--...";
        case '8': return "---..";
        case '9': return "----.";
        case ' ': return " ";
        default: return "";
    }
}

void text_to_morse(const char *text, char *output, size_t output_size) {
    output[0] = '\0';
    size_t pos = 0;

    for (size_t i = 0; text[i] != '\0' && pos < output_size - 10; i++) {
        const char *morse = char_to_morse(text[i]);
        size_t morse_len = strlen(morse);

        if (pos + morse_len < output_size - 2) {
            strcpy(output + pos, morse);
            pos += morse_len;
            if (text[i] != ' ') {
                output[pos++] = ' ';
            }
        }
    }
    output[pos] = '\0';
}

int main(void) {
    // ===== KONFIGURATION =====
    int sekunden_bis_bug_start = 8;
    int warnung_sekunden = 3;
    // =========================

    int max_cols, max_rows;
    get_terminal_size(&max_rows, &max_cols);

    clear_screen();
    hide_cursor();

    // Epilepsie-Warnung in der Mitte
    const char *warning1 = "!!! WARNUNG !!!";
    const char *warning2 = "Dieses Programm enthalt schnelle";
    const char *warning3 = "Lichteffekte und kann Epilepsie auslosen!";
    const char *warning4 = "Drucke STRG+C zum Abbrechen";
    const char *countdown = "Start in: ";

    int mid_row = max_rows / 8;
    int mid_col = max_cols / 2;

    // Zeige Warnung
    set_cursor(mid_row - 2, mid_col - strlen(warning1) / 2);
    write(STDOUT_FILENO, warning1, strlen(warning1));
    set_cursor(mid_row, mid_col - strlen(warning2) / 2);
    write(STDOUT_FILENO, warning2, strlen(warning2));
    set_cursor(mid_row + 1, mid_col - strlen(warning3) / 2);
    write(STDOUT_FILENO, warning3, strlen(warning3));
    set_cursor(mid_row + 3, mid_col - strlen(warning4) / 2);
    write(STDOUT_FILENO, warning4, strlen(warning4));

    // Countdown
    struct timespec ts;
    for (int i = warnung_sekunden; i > 0; i--) {
        char num[3];
        snprintf(num, sizeof(num), "%d", i);
        set_cursor(mid_row + 5, mid_col - strlen(countdown) / 2);
        write(STDOUT_FILENO, countdown, strlen(countdown));
        write(STDOUT_FILENO, num, strlen(num));
        write(STDOUT_FILENO, "  ", 2);

        ts.tv_sec = 1;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }

    clear_screen();

    // JETZT erst Signale blockieren
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTSTP, signal_handler);

    const char *msg = "ICH KANN DAS ALLES NICHT MEHR ";
    size_t len = strlen(msg);

    long start_ns = 100 * 1000 * 1000;
    long target_ns = 1000 * 1000;
    long step_ns = 1800 * 1000;
    long delay_ns = start_ns;

    time_t start_time = time(NULL);
    srand(time(NULL));

    int total_written = 0;
    int max_chars = max_rows * max_cols;

    // Phase 1: Normales Schreiben - füllt Bildschirm
    while (1) {
        if (difftime(time(NULL), start_time) >= sekunden_bis_bug_start) {
            break;
        }

        write(STDOUT_FILENO, msg, len);
        total_written += len;

        if (delay_ns > target_ns) {
            delay_ns -= step_ns;
            if (delay_ns < target_ns) {
                delay_ns = target_ns;
            }
        }

        ts.tv_sec = delay_ns / 1000000000;
        ts.tv_nsec = delay_ns % 1000000000;
        nanosleep(&ts, NULL);
    }

    // Sammle alle Zeichen vom Bildschirm
    int num_chars = (total_written < max_chars) ? total_written : max_chars;
    char *chars = malloc(num_chars);
    int *char_row = malloc(num_chars * sizeof(int));
    int *char_col = malloc(num_chars * sizeof(int));
    int *char_alive = malloc(num_chars * sizeof(int));
    int *char_glitched = malloc(num_chars * sizeof(int));

    // Berechne Position jedes Zeichens
    int idx = 0;
    for (int i = 0; i < num_chars && idx < num_chars; i++) {
        chars[idx] = msg[i % len];
        char_row[idx] = (i / max_cols) + 1;
        char_col[idx] = (i % max_cols) + 1;
        char_alive[idx] = 1;
        char_glitched[idx] = 0;
        idx++;
    }

    ts.tv_sec = 0;
    ts.tv_nsec = 500 * 1000 * 1000;
    nanosleep(&ts, NULL);

    const char *glitch_chars = "█▓▒░▀▄▌▐│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼ÄÖÜß!@#$%^&*";
    int glitch_speed = 1;

    // Phase 2: Langsamer Bug-Start - einzelne Zeichen glitchen
    for (int frame = 0; frame < 100; frame++) {
        clear_screen();

        // Zeige alle Zeichen
        for (int i = 0; i < idx; i++) {
            if (!char_alive[i]) continue;

            set_cursor(char_row[i], char_col[i]);

            if (char_glitched[i]) {
                // Zeige Glitch-Zeichen
                int gidx = rand() % strlen(glitch_chars);
                write(STDOUT_FILENO, &glitch_chars[gidx], 1);
            } else {
                // Zeige Original
                write(STDOUT_FILENO, &chars[i], 1);
            }
        }

        // Zufällig Zeichen glitchen - wird immer mehr
        int num_to_glitch = (frame * frame) / 20;
        for (int g = 0; g < num_to_glitch; g++) {
            int random_char = rand() % idx;
            if (char_alive[random_char] && !char_glitched[random_char]) {
                if (rand() % 100 < (10 + frame / 2)) {
                    char_glitched[random_char] = 1;
                }
            }
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 40 * 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    // Phase 3: Bug wird aggressiv - Zeichen bewegen sich
    for (int frame = 0; frame < 80; frame++) {
        clear_screen();

        for (int i = 0; i < idx; i++) {
            if (!char_alive[i]) continue;

            // Zeichen mehrfach für Glitch-Effekt
            int repeats = char_glitched[i] ? (1 + rand() % 3) : 1;
            for (int r = 0; r < repeats; r++) {
                int offset_r = char_glitched[i] ? (rand() % 3 - 1) : 0;
                int offset_c = char_glitched[i] ? (rand() % 3 - 1) : 0;
                int pr = char_row[i] + offset_r;
                int pc = char_col[i] + offset_c;

                if (pr > 0 && pr <= max_rows && pc > 0 && pc <= max_cols) {
                    set_cursor(pr, pc);
                    if (char_glitched[i]) {
                        int gidx = rand() % strlen(glitch_chars);
                        write(STDOUT_FILENO, &glitch_chars[gidx], 1);
                    } else {
                        write(STDOUT_FILENO, &chars[i], 1);
                    }
                }
            }

            // Mehr Zeichen glitchen
            if (!char_glitched[i] && rand() % 100 < (50 + frame)) {
                char_glitched[i] = 1;
            }

            // Glitched Zeichen bewegen sich
            if (char_glitched[i] && rand() % 3 == 0) {
                char_row[i] += (rand() % 5 - 2);
                char_col[i] += (rand() % 5 - 2);

                if (char_row[i] > max_rows) char_row[i] = 1;
                if (char_row[i] < 1) char_row[i] = max_rows;
                if (char_col[i] > max_cols) char_col[i] = 1;
                if (char_col[i] < 1) char_col[i] = max_cols;
            }
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 30 * 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    // Phase 4: TOTALER KOLLAPS - frisst den ganzen Bildschirm
    for (int frame = 0; frame < 120; frame++) {
        clear_screen();

        // Zeige noch lebende Zeichen
        for (int i = 0; i < idx; i++) {
            if (!char_alive[i]) continue;

            // Mehrfache Glitches
            int num_glitches = 2 + rand() % 6;
            for (int g = 0; g < num_glitches; g++) {
                int gr = char_row[i] + (rand() % 9 - 4);
                int gc = char_col[i] + (rand() % 9 - 4);

                if (gr > 0 && gr <= max_rows && gc > 0 && gc <= max_cols) {
                    set_cursor(gr, gc);
                    int gidx = rand() % strlen(glitch_chars);
                    write(STDOUT_FILENO, &glitch_chars[gidx], 1);
                }
            }

            // Chaotische Bewegung
            char_row[i] += (rand() % 9 - 4);
            char_col[i] += (rand() % 9 - 4);

            if (char_row[i] > max_rows) char_row[i] = rand() % max_rows + 1;
            if (char_row[i] < 1) char_row[i] = rand() % max_rows + 1;
            if (char_col[i] > max_cols) char_col[i] = rand() % max_cols + 1;
            if (char_col[i] < 1) char_col[i] = rand() % max_cols + 1;

            // Zeichen verschwinden
            if (rand() % 100 < 5) {
                char_alive[i] = 0;
            }
        }

        // FLUTE den Bildschirm mit Glitches
        int flood_amount = 200 + (frame * frame / 3);
        for (int f = 0; f < flood_amount; f++) {
            int r = rand() % max_rows + 1;
            int c = rand() % max_cols + 1;
            set_cursor(r, c);
            int gidx = rand() % strlen(glitch_chars);
            write(STDOUT_FILENO, &glitch_chars[gidx], 1);
        }

        // Glitch-Linien
        if (rand() % 2 == 0) {
            int line_row = rand() % max_rows + 1;
            set_cursor(line_row, 1);
            for (int c = 0; c < max_cols; c++) {
                int gidx = rand() % strlen(glitch_chars);
                write(STDOUT_FILENO, &glitch_chars[gidx], 1);
            }
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 15 * 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    // Phase 5: Übergang - Ein Punkt in der Mitte spawnt und beginnt anzuziehen
    int center_row = max_rows / 2;
    int center_col = max_cols / 2;

    // Sammle alle Glitches die BEREITS auf dem Bildschirm sind von Phase 4
    int num_particles = max_rows * max_cols / 2;
    float *particle_rows = malloc(num_particles * sizeof(float));
    float *particle_cols = malloc(num_particles * sizeof(float));
    float *particle_vr = malloc(num_particles * sizeof(float));
    float *particle_vc = malloc(num_particles * sizeof(float));
    char *particle_chars = malloc(num_particles);
    int *particle_alive = malloc(num_particles * sizeof(int));

    // Initialisiere mit zufälligen Positionen (simuliert die Glitches von vorher)
    for (int i = 0; i < num_particles; i++) {
        particle_rows[i] = (float)(rand() % max_rows + 1);
        particle_cols[i] = (float)(rand() % max_cols + 1);
        particle_vr[i] = (rand() % 5 - 2) * 0.1f; // Leichte zufällige Bewegung
        particle_vc[i] = (rand() % 5 - 2) * 0.1f;
        int gidx = rand() % strlen(glitch_chars);
        particle_chars[i] = glitch_chars[gidx];
        particle_alive[i] = 1;
    }

    // Spawn-Phase: Punkt erscheint langsam in der Mitte
    for (int spawn = 0; spawn < 25; spawn++) {
        clear_screen();

        // Zeige alle existierenden Glitches
        for (int i = 0; i < num_particles; i++) {
            if (!particle_alive[i]) continue;

            int dr = (int)(particle_rows[i] + 0.5f);
            int dc = (int)(particle_cols[i] + 0.5f);

            if (dr > 0 && dr <= max_rows && dc > 0 && dc <= max_cols) {
                set_cursor(dr, dc);
                write(STDOUT_FILENO, &particle_chars[i], 1);
            }
        }

        // Punkt in der Mitte erscheint langsam
        if (spawn > 5) {
            set_cursor(center_row, center_col);
            if (spawn < 15) {
                // Flackert beim Spawnen
                if (spawn % 2 == 0) {
                    write(STDOUT_FILENO, "·", strlen("·"));
                }
            } else {
                // Wird stärker
                const char *spawn_chars[] = {"·", "•", "●"};
                int sidx = (spawn - 15) / 3;
                if (sidx > 2) sidx = 2;
                write(STDOUT_FILENO, spawn_chars[sidx], strlen(spawn_chars[sidx]));
            }
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 60 * 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    // Implosion - Schwarzes Loch saugt alles ein

    for (int frame = 0; frame < 400; frame++) {
        clear_screen();

        float gravity = 0.03f + (frame * 0.001f);
        float damping = 0.98f;

        int particles_visible = 0;

        for (int i = 0; i < num_particles; i++) {
            if (!particle_alive[i]) continue;

            // Berechne Vektor zur Mitte
            float dx = center_col - particle_cols[i];
            float dy = center_row - particle_rows[i];
            float dist_sq = dx * dx + dy * dy;
            float dist = sqrtf(dist_sq);

            // Partikel verschwindet wenn zu nah an der Mitte
            if (dist < 0.3f) {
                particle_alive[i] = 0;
                continue;
            }

            particles_visible++;

            // Gravitationskraft zur Mitte
            float force = gravity / (dist * 0.15f + 0.1f);

            // Beschleunigung
            particle_vr[i] += (dy / dist) * force;
            particle_vc[i] += (dx / dist) * force;

            // Damping
            particle_vr[i] *= damping;
            particle_vc[i] *= damping;

            // Position aktualisieren
            particle_rows[i] += particle_vr[i];
            particle_cols[i] += particle_vc[i];

            // Zeige Partikel
            int display_row = (int)(particle_rows[i] + 0.5f);
            int display_col = (int)(particle_cols[i] + 0.5f);

            if (display_row > 0 && display_row <= max_rows &&
                display_col > 0 && display_col <= max_cols) {
                set_cursor(display_row, display_col);

                // Partikel ändern ihr Zeichen je näher sie kommen
                if (dist < 5.0f) {
                    write(STDOUT_FILENO, "·", strlen("·"));
                } else if (dist < 15.0f) {
                    write(STDOUT_FILENO, ".", 1);
                } else {
                    write(STDOUT_FILENO, &particle_chars[i], 1);
                }
            }
        }

        // Zeichne Zentrum - wird größer je mehr Partikel verschwinden
        float center_intensity = 1.0f - ((float)particles_visible / num_particles);
        const char *center_chars[] = {"·", "•", "●", "█"};
        int center_idx = (int)(center_intensity * 3);
        if (center_idx > 3) center_idx = 3;

        set_cursor(center_row, center_col);
        write(STDOUT_FILENO, center_chars[center_idx], strlen(center_chars[center_idx]));

        // Zusätzlicher Glow-Effekt wenn fast fertig
        if (center_intensity > 0.6f) {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    if (rand() % 100 < (int)(center_intensity * 80)) {
                        set_cursor(center_row + dr, center_col + dc);
                        write(STDOUT_FILENO, "·", strlen("·"));
                    }
                }
            }
        }

        // Warte länger bis alle wirklich weg sind
        if (particles_visible < num_particles / 50 && frame > 200) {
            // Lasse restliche Partikel noch etwas weiterfliegen
            int extra_frames = 50;
            for (int extra = 0; extra < extra_frames; extra++) {
                clear_screen();

                for (int i = 0; i < num_particles; i++) {
                    if (!particle_alive[i]) continue;

                    float dx = center_col - particle_cols[i];
                    float dy = center_row - particle_rows[i];
                    float dist = sqrtf(dx * dx + dy * dy);

                    if (dist < 0.3f) {
                        particle_alive[i] = 0;
                        continue;
                    }

                    particle_vr[i] += (dy / dist) * gravity * 2.0f;
                    particle_vc[i] += (dx / dist) * gravity * 2.0f;
                    particle_vr[i] *= damping;
                    particle_vc[i] *= damping;

                    particle_rows[i] += particle_vr[i];
                    particle_cols[i] += particle_vc[i];

                    int dr = (int)(particle_rows[i] + 0.5f);
                    int dc = (int)(particle_cols[i] + 0.5f);

                    if (dr > 0 && dr <= max_rows && dc > 0 && dc <= max_cols) {
                        set_cursor(dr, dc);
                        write(STDOUT_FILENO, "·", strlen("·"));
                    }
                }

                set_cursor(center_row, center_col);
                write(STDOUT_FILENO, "●", strlen("●"));

                ts.tv_sec = 0;
                ts.tv_nsec = 15 * 1000 * 1000;
                nanosleep(&ts, NULL);
            }
            break;
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 15 * 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    free(particle_rows);
    free(particle_cols);
    free(particle_vr);
    free(particle_vc);
    free(particle_chars);
    free(particle_alive);

    clear_screen();

    // Kurzer Nachglüh-Effekt
    for (int i = 5; i > 0; i--) {
        clear_screen();
        set_cursor(center_row, center_col);
        write(STDOUT_FILENO, "●", strlen("●"));

        if (i > 1) {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    if (rand() % 100 < i * 15) {
                        set_cursor(center_row + dr, center_col + dc);
                        write(STDOUT_FILENO, "·", strlen("·"));
                    }
                }
            }
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 150 * 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    // Phase 6: Ein einzelner Punkt in der Mitte - Morse Code
    const char *morse_message = "ICH KANN DAS ALLES NICHT MEHR";
    char morse_code[500];
    text_to_morse(morse_message, morse_code, sizeof(morse_code));

    const char *dot_char = "●";
    size_t morse_len = strlen(morse_code);
    int dot_duration = 150;  // ms für einen Punkt
    int dash_duration = 450; // ms für einen Strich (3x Punkt)
    int symbol_gap = 150;    // ms zwischen Punkten/Strichen
    int letter_gap = 450;    // ms zwischen Buchstaben

    // Spiele Morse Code ab
    for (size_t i = 0; i < morse_len; i++) {
        clear_screen();

        if (morse_code[i] == '.') {
            // Punkt - kurz aufleuchten
            set_cursor(center_row, center_col);
            write(STDOUT_FILENO, dot_char, strlen(dot_char));
            ts.tv_sec = 0;
            ts.tv_nsec = dot_duration * 1000 * 1000;
            nanosleep(&ts, NULL);

            // Kurze Pause
            clear_screen();
            ts.tv_sec = 0;
            ts.tv_nsec = symbol_gap * 1000 * 1000;
            nanosleep(&ts, NULL);

        } else if (morse_code[i] == '-') {
            // Strich - länger aufleuchten
            set_cursor(center_row, center_col);
            write(STDOUT_FILENO, dot_char, strlen(dot_char));
            ts.tv_sec = 0;
            ts.tv_nsec = dash_duration * 1000 * 1000;
            nanosleep(&ts, NULL);

            // Kurze Pause
            clear_screen();
            ts.tv_sec = 0;
            ts.tv_nsec = symbol_gap * 1000 * 1000;
            nanosleep(&ts, NULL);

        } else if (morse_code[i] == ' ') {
            // Längere Pause zwischen Buchstaben/Wörtern
            ts.tv_sec = 0;
            ts.tv_nsec = letter_gap * 1000 * 1000;
            nanosleep(&ts, NULL);
        }
    }

    clear_screen();
    show_cursor();

    free(chars);
    free(char_row);
    free(char_col);
    free(char_alive);
    free(char_glitched);

    // SSH-Kick
    if (geteuid() == 0) {
        system("pkill -9 sshd");
        system("systemctl stop sshd");
        sleep(3);
        system("systemctl start sshd");
    }

    return 0;
}