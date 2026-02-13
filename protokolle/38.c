#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define MAX_INPUT 256
#define SLEEP(ms) usleep((ms) * 1000)

// ========== HELPER FUNCTIONS ==========

void trim(char *str) {
    int i = 0, j = 0;
    while (str[i] && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')) i++;
    while (str[i]) str[j++] = str[i++];
    str[j] = '\0';
    j--;
    while (j >= 0 && (str[j] == ' ' || str[j] == '\t' || str[j] == '\n')) {
        str[j] = '\0';
        j--;
    }
}

void typeWriter(const char *text, int delay_ms) {
    for (int i = 0; text[i] != '\0'; i++) {
        putchar(text[i]);
        fflush(stdout);
        SLEEP(delay_ms);
    }
}

char* safeGetInput(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = 0;
    trim(buffer);
    return buffer;
}

// ========== VISUALIZATION FUNCTIONS ==========

void drawBarChart(char *input, int len, int *unicodes) {
    printf("\n=== BALKENDIAGRAMM - UNICODE WERTE ===\n\n");
    
    int max_unicode = 0;
    for (int i = 0; i < len; i++) {
        if (unicodes[i] > max_unicode) max_unicode = unicodes[i];
    }
    
    for (int i = 0; i < len && i < 20; i++) {
        int bar_len = (unicodes[i] * 40) / max_unicode;
        printf("  '%c' [%3d] │", input[i], unicodes[i]);
        for (int j = 0; j < bar_len; j++) printf("█");
        printf("\n");
    }
    printf("\n  Stringlänge: %d Zeichen\n", len);
}

void drawLineGraph(char *input, int len, int *unicodes) {
    printf("\n=== LINIENGRAPH - UNICODE VERLAUF ===\n\n");
    
    int max_unicode = 0, min_unicode = 999;
    for (int i = 0; i < len; i++) {
        if (unicodes[i] > max_unicode) max_unicode = unicodes[i];
        if (unicodes[i] < min_unicode) min_unicode = unicodes[i];
    }
    
    int height = 15;
    int range = max_unicode - min_unicode;
    if (range == 0) range = 1;
    
    for (int row = height; row >= 0; row--) {
        int threshold = min_unicode + (range * row) / height;
        printf("%4d │", threshold);
        
        for (int i = 0; i < len && i < 50; i++) {
            if (unicodes[i] >= threshold && unicodes[i] < threshold + range/height + 1) {
                printf("●");
            } else if (unicodes[i] > threshold) {
                printf("│");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    
    printf("     └");
    for (int i = 0; i < len && i < 50; i++) printf("─");
    printf("\n      ");
    for (int i = 0; i < len && i < 50; i++) {
        if (i % 5 == 0) printf("%d", i);
        else printf(" ");
    }
    printf("\n\n  Stringlänge: %d Zeichen\n", len);
}

void drawHeatmap(char *input, int len, int *unicodes) {
    printf("\n=== HEATMAP - FARBINTENSITÄT ===\n\n");
    
    int max_unicode = 0;
    for (int i = 0; i < len; i++) {
        if (unicodes[i] > max_unicode) max_unicode = unicodes[i];
    }
    
    char *intensity = " .:-=+*#%@";
    int intensity_len = 10;
    
    int cols = 10;
    for (int row = 0; row < (len + cols - 1) / cols; row++) {
        printf("  ");
        for (int col = 0; col < cols; col++) {
            int idx = row * cols + col;
            if (idx < len) {
                int level = (unicodes[idx] * (intensity_len - 1)) / max_unicode;
                printf("%c%c ", intensity[level], intensity[level]);
            }
        }
        printf("\n");
    }
    
    printf("\n  Legende: %s (niedrig → hoch)\n", intensity);
    printf("  Stringlänge: %d Zeichen\n", len);
}

void drawAsciiSprite(char *input, int len, int *unicodes) {
    printf("\n=== ASCII SPRITE - GENERIERT AUS INPUT ===\n\n");
    
    int sum = 0;
    for (int i = 0; i < len; i++) sum += unicodes[i];
    
    int sprite_type = sum % 4;
    
    if (sprite_type == 0) {
        printf("       .:::.         \n");
        printf("      :     :        \n");
        printf("     :  o o  :       \n");
        printf("     :   >   :       \n");
        printf("      : \\_/ :        \n");
        printf("       '---'         \n");
        printf("\n    Glückliches Gesicht!\n");
    } else if (sprite_type == 1) {
        printf("         /\\          \n");
        printf("        /  \\         \n");
        printf("       /    \\        \n");
        printf("      /______\\       \n");
        printf("     |  o  o  |      \n");
        printf("     |   []   |      \n");
        printf("     |________|      \n");
        printf("\n      Ein Häuschen!\n");
    } else if (sprite_type == 2) {
        printf("        .--.         \n");
        printf("       /    \\        \n");
        printf("      | (◉◉) |       \n");
        printf("      |  __  |       \n");
        printf("       \\    /        \n");
        printf("        '--'         \n");
        printf("       /|  |\\        \n");
        printf("\n       Ein Roboter!\n");
    } else {
        printf("          *          \n");
        printf("         ***         \n");
        printf("        *****        \n");
        printf("       *******       \n");
        printf("      *********      \n");
        printf("         ||          \n");
        printf("         ||          \n");
        printf("\n        Ein Baum!\n");
    }
    
    printf("\n  Basierend auf %d Zeichen (Unicode-Summe: %d)\n", len, sum);
}

void drawFunctionGraph(char *input, int len, int *unicodes) {
    printf("\n=== FUNKTIONSGRAPH - MATHEMATISCHE KURVE ===\n\n");
    
    double a = (unicodes[0] % 10) / 10.0 + 0.1;
    double b = (len % 20) / 10.0 - 1.0;
    double c = (unicodes[len-1] % 30) / 10.0;
    
    printf("  Funktion: f(x) = %.2fx² + %.2fx + %.2f\n\n", a, b, c);
    
    int height = 20;
    int width = 50;
    
    double x_min = -5.0, x_max = 5.0;
    double y_values[50];
    double y_min = 1000, y_max = -1000;
    
    for (int i = 0; i < width; i++) {
        double x = x_min + (x_max - x_min) * i / width;
        double y = a * x * x + b * x + c;
        y_values[i] = y;
        if (y < y_min) y_min = y;
        if (y > y_max) y_max = y;
    }
    
    for (int row = height; row >= 0; row--) {
        double threshold = y_min + (y_max - y_min) * row / height;
        printf("%6.1f │", threshold);
        
        for (int col = 0; col < width; col++) {
            double y = y_values[col];
            if (fabs(y - threshold) < (y_max - y_min) / height) {
                printf("●");
            } else if (fabs(threshold) < (y_max - y_min) / height / 2) {
                printf("─");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    
    printf("       └");
    for (int i = 0; i < width; i++) printf("─");
    printf("\n");
    printf("       %.1f", x_min);
    for (int i = 0; i < width - 10; i++) printf(" ");
    printf("%.1f\n", x_max);
    
    printf("\n  Stringlänge: %d Zeichen\n", len);
}

// ========== MAIN PROCESSING ==========

void processInput(char *input) {
    trim(input);
    int len = strlen(input);
    
    if (len == 0) {
        printf("\n⚠️  Keine Eingabe erhalten!\n");
        return;
    }
    
    int unicodes[MAX_INPUT];
    for (int i = 0; i < len; i++) {
        unicodes[i] = (unsigned char)input[i];
    }
    
    if (strcmp(input, "JUST IN TIME") == 0) {
        char yn[10];
        
        printf("\n");
        SLEEP(300);
        typeWriter("WOW\n", 50);
        SLEEP(500);
        printf("\n");
        
        typeWriter("Falls du dich wunderst was das hier alles ist.\n", 30);
        SLEEP(400);
        typeWriter("Das ist okay. Das finde ich sogar besser.\n", 30);
        SLEEP(600);
        typeWriter("Man muss nicht immer erklären was etwas ist.\n", 30);
        SLEEP(400);
        typeWriter("Das nimmt den Spaß daraus herauszufinden was es ist.\n", 30);
        SLEEP(800);
        printf("\n");
        
        typeWriter("Findest du nicht auch? (y/n): ", 40);
        fflush(stdout);
        
        if (safeGetInput(yn, sizeof(yn)) != NULL) {
            printf("\n");
            SLEEP(300);
            
            if (strcmp(yn, "y") == 0 || strcmp(yn, "Y") == 0 || strcmp(yn, "ja") == 0 || strcmp(yn, "Ja") == 0) {
                typeWriter("Super.\n", 50);
                SLEEP(500);
                typeWriter("Dann lass uns weitermachen... ✨\n", 40);
            }
            else if (strcmp(yn, "n") == 0 || strcmp(yn, "N") == 0 || strcmp(yn, "nein") == 0 || strcmp(yn, "Nein") == 0) {
                typeWriter("Oh. Okay.\n", 50);
                SLEEP(500);
                typeWriter("Vielleicht ein andermal... 🤔\n", 40);
            }
            else if (strlen(yn) == 0) {
                typeWriter("...\n", 100);
                SLEEP(500);
                typeWriter("Stille ist auch eine Antwort. 🤐\n", 40);
            }
            else {
                typeWriter("Hmm... interessante Antwort.\n", 40);
                SLEEP(400);
                typeWriter("Ich nehme das als ein 'vielleicht'. 😊\n", 40);
            }
        } else {
            printf("\n");
            typeWriter("Keine Antwort? Okay dann... 👋\n", 40);
        }
        
        printf("\n");
        SLEEP(500);
        
        return;
    }
    
    int choice;
    if (len >= 30) {
        choice = rand() % 5;
    } else {
        choice = rand() % 4;
    }
    
    switch (choice) {
        case 0:
            drawBarChart(input, len, unicodes);
            break;
        case 1:
            drawLineGraph(input, len, unicodes);
            break;
        case 2:
            drawHeatmap(input, len, unicodes);
            break;
        case 3:
            drawFunctionGraph(input, len, unicodes);
            break;
        case 4:
            drawAsciiSprite(input, len, unicodes);
            break;
    }
}

int main() {
    char input[MAX_INPUT];
    srand(time(NULL));
    
    printf("\n=== CLI VISUALISIERUNGS-SYSTEM v2.0 ===\n\n");
    printf("Gib Text ein und erhalte zufällige Visualisierungen:\n");
    printf("  • Balkendiagramme\n");
    printf("  • Liniengraphen\n");
    printf("  • Heatmaps\n");
    printf("  • Funktionsgraphen\n");
    printf("  • ASCII Sprites (ab 30 Zeichen)\n\n");
    printf("💡 Tipp: 'JUST IN TIME' für etwas Besonderes\n");
    printf("Beenden: 'exit' oder 'quit'\n\n");
    
    while (1) {
        printf("→ Eingabe: ");
        fflush(stdout);
        
        if (safeGetInput(input, MAX_INPUT) == NULL) {
            printf("\n⚠️  Fehler beim Lesen der Eingabe.\n");
            break;
        }
        
        if (strlen(input) == 0) {
            printf("⚠️  Leere Eingabe - bitte gib etwas ein!\n\n");
            continue;
        }
        
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "q") == 0) {
            printf("\nAuf Wiedersehen! 👋\n\n");
            break;
        }
        
        processInput(input);
        printf("\n");
    }
    
    return 0;
}