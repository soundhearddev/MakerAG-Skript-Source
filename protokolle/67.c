#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define KEY "MEINSCHLUESSEL"

int main() {
    int keylen = strlen(KEY);
    int keyidx = 0;
    int c;

    // Lies Input von stdin
    while ((c = getchar()) != EOF) {
        if (isalpha(c)) {
            int base = isupper(c) ? 'A' : 'a';
            int k = toupper(KEY[keyidx % keylen]) - 'A';
            // Entschlüsselung: P = (C - K + 26) % 26
            c = ((c - base - k + 26) % 26) + base;
            keyidx++;
        }
        putchar(c); // Ausgabe direkt auf stdout
    }

    return 0;
}
