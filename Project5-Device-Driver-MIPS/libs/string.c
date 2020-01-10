#include "string.h"

int strlen(char *src) {
    int i;
    for (i = 0; src[i] != '\0'; i++) {
    }
    return i;
}

void memcpy(uint8_t *dest, uint8_t *src, uint32_t len) {
    for (; len != 0; len--) {
        *dest++ = *src++;
    }
}

void memset(void *dest, uint8_t val, uint32_t len) {
    uint8_t *dst = (uint8_t *) dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
}

void bzero(void *dest, uint32_t len) {
    memset(dest, 0, len);
}

int strcmp(char *str1, char *str2) {
    while (*str1 && *str2 && (*str1++ == *str2++)) {
    };

    return (*str1) - (*str2);
}

int memcmp(char *str1, char *str2, uint32_t size) {
    int i;

    for (i = 0; i < size; i++) {
        if (str1[i] > str2[i]) {
            return 1;
        } else if (str1[i] < str2[i]) {
            return -1;
        }
    }
    return 0;
}

char *strcpy(char *dest, char *src) {
    int l = strlen(src);
    int i;

    for (i = 0; i < l; i++) {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}

int atoi(char *str) {
    int l = strlen(str);
    int ans = 0, i;

    for (i = 0; i < l; i++)
        ans = ans * 10 + str[i] - '0';

    return ans;
}

int chartoi(char ch) {

    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

int htoi(char *str) {
    if (str[0] != '0' || str[1] != 'x') {
        return -1;
    }
    int l = strlen(str);
    int ans = 0, i;
    for (i = 2; i < l; i++) {
        ans = ans * 16 + chartoi(str[i]);
    }
    return ans;
}