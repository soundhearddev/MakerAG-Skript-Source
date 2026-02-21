#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define MAX_INPUT 256
#define SLEEP(ms) usleep((ms) * 1000)

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
    if (fgets(buffer, size, stdin) == NULL) return NULL;
    buffer[strcspn(buffer, "\n")] = 0;
    trim(buffer);
    return buffer;
}

void drawFunctionGraph(char *input, int len, int *unicodes) {
    double a = (unicodes[0] % 10) / 10.0 + 0.1;
    double b = (len % 20) / 10.0 - 1.0;
    double c = (unicodes[len-1] % 30) / 10.0;

    printf("\n  f(x) = %.2fx\xc2\xb2 + %.2fx + %.2f\n\n", a, b, c);

    int height = 20;
    int width  = 60;

    double x_min = -5.0, x_max = 5.0;
    double y_values[60];
    double y_min = 1e9, y_max = -1e9;

    for (int i = 0; i < width; i++) {
        double x = x_min + (x_max - x_min) * i / (width - 1);
        double y = a*x*x + b*x + c;
        y_values[i] = y;
        if (y < y_min) y_min = y;
        if (y > y_max) y_max = y;
    }

    double y_range = y_max - y_min;
    if (y_range < 1e-9) y_range = 1.0;

    /* find x-axis row */
    int zero_row = -1;
    if (y_min <= 0.0 && y_max >= 0.0)
        zero_row = (int)round((double)height * (y_max / y_range));

    for (int row = height; row >= 0; row--) {
        double threshold = y_min + y_range * row / height;
        printf("%7.2f \xe2\x94\x82", threshold);   /* │ */

        int is_zero_row = (row == zero_row);

        /* find y-axis column (x=0) */
        int zero_col = -1;
        if (x_min <= 0.0 && x_max >= 0.0)
            zero_col = (int)round((double)(width - 1) * (-x_min) / (x_max - x_min));

        for (int col = 0; col < width; col++) {
            double y = y_values[col];
            int y_row = (int)round((double)height * (y - y_min) / y_range);

            if (y_row == row) {
                /* curve point — use + at intersection with axes */
                if (col == zero_col && is_zero_row)
                    printf("\xe2\x94\xbc"); /* ┼ */
                else
                    printf("\xe2\x97\x8f"); /* ● */
            } else if (is_zero_row && col == zero_col) {
                printf("\xe2\x94\xbc"); /* ┼ */
            } else if (is_zero_row) {
                printf("\xe2\x94\x80"); /* ─ */
            } else if (col == zero_col) {
                printf("\xe2\x94\x82"); /* │ */
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    /* bottom axis */
    printf("        \xe2\x94\x94");   /* └ */
    for (int i = 0; i < width; i++) printf("\xe2\x94\x80");
    printf("\n");

    /* x labels */
    printf("        %.1f", x_min);
    int mid = width / 2 - 2;
    for (int i = 0; i < mid; i++) printf(" ");
    printf("0.0");
    for (int i = 0; i < mid - 1; i++) printf(" ");
    printf("%.1f\n", x_max);

    printf("\n  Stringl\xc3\xa4nge: %d Zeichen\n", len);
}

void processInput(char *input) {
    trim(input);
    int len = strlen(input);

    if (len == 0) { printf("\nKeine Eingabe!\n"); return; }

    int unicodes[MAX_INPUT];
    for (int i = 0; i < len; i++) unicodes[i] = (unsigned char)input[i];

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
        typeWriter("Man muss nicht immer erkl\xc3\xa4ren was etwas ist.\n", 30);
        SLEEP(400);
        typeWriter("Das nimmt den Spa\xc3\x9f daraus herauszufinden was es ist.\n", 30);
        SLEEP(800);
        printf("\n");
        typeWriter("Findest du nicht auch? (y/n): ", 40);
        fflush(stdout);
        if (safeGetInput(yn, sizeof(yn)) != NULL) {
            printf("\n"); SLEEP(300);
            if (strcmp(yn,"y")==0||strcmp(yn,"Y")==0||strcmp(yn,"ja")==0||strcmp(yn,"Ja")==0) {
                typeWriter("Super.\n", 50); SLEEP(500);
                typeWriter("Dann lass uns weitermachen... \xe2\x9c\xa8\n", 40);
            } else if (strcmp(yn,"n")==0||strcmp(yn,"N")==0||strcmp(yn,"nein")==0||strcmp(yn,"Nein")==0) {
                typeWriter("Oh. Okay.\n", 50); SLEEP(500);
                typeWriter("Vielleicht ein andermal... \n", 40);
            } else if (strlen(yn)==0) {
                typeWriter("...\n", 100); SLEEP(500);
                typeWriter("Stille ist auch eine Antwort. \n", 40);
            } else {
                typeWriter("Hmm... interessante Antwort.\n", 40); SLEEP(400);
                typeWriter("Ich nehme das als ein 'vielleicht'. \n", 40);
            }
        }
        printf("\n"); SLEEP(500);
        return;
    }

    drawFunctionGraph(input, len, unicodes);
}

int main() {
    char input[MAX_INPUT];
    srand(time(NULL));

    while (1) {
        printf("\n\xe2\x86\x92 Eingabe: ");
        fflush(stdout);

        if (safeGetInput(input, MAX_INPUT) == NULL) {
            printf("\nFehler beim Lesen.\n");
            break;
        }
        if (strlen(input) == 0) {
            printf("Leere Eingabe - bitte gib etwas ein!\n");
            continue;
        }
        if (strcmp(input,"exit")==0||strcmp(input,"quit")==0||strcmp(input,"q")==0) {
            printf("\nAuf Wiedersehen!\n\n");
            break;
        }

        processInput(input);
        printf("\n");
    }
    return 0;
}