#include <stdio.h>
#include <string.h>  // nötig für strlen

int main() {
    // Das ist der Text, den man pipen kann
    const char *hidden_text = "\n\n\n tetyg ypp aizlw tol pntg ctyp rm xfz ejrj kjs xifci omw mf fqjs zyfcxtarqrjp dtlh owry mptrk ibe fäyxl \n\n\n";

    // Schreibe den Text direkt auf stdout
    fwrite(hidden_text, 1, strlen(hidden_text), stdout);

    return 0;
}
