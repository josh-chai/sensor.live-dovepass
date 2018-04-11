/*
    file: codes.c encrypt/decrypt
*/

#include <stdio.h>
#include <string.h>

char *code_tab = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void dove_encode(char *str, char *key, char *res)
{
    int idx;
    unsigned int ch, first_4bit, last_4bit;
    char code, ch1, ch2;

    while(*str != '\0') {
        ch = (unsigned int) *str;
        first_4bit = ch / 16;
        last_4bit = ch % 16;
        ch1 = *(code_tab + first_4bit);
        ch2 = *(code_tab + last_4bit + 16);

        *res = ch1;
        ++res;
        *res = ch2;
        ++res;
        ++str;
    }

    *res = '\0';
}


void dove_decode(char *str, char *key, char *res)
{
    int idx;
    unsigned int ch, first_4bit, last_4bit;
    char code, buf[2];

    buf[1] = '\0';

    while(*str != '\0') {
        buf[0] = *str;
        first_4bit = strstr(code_tab, buf) - code_tab;
        ++str;

        buf[0] = *str;
        last_4bit = strstr(code_tab, buf) - code_tab - 16;
        ch = first_4bit * 16 + last_4bit;
        code = (char) ch;
        ++str;

        *res = code;
        ++res;
    }
    *res = '\0';
}