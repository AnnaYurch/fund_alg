#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *convert_binaryTo_2R(char *binary, int r) {
    int len = strlen(binary);
    int numGroups = (len + r - 1) / r;
    char *result = (char *)malloc(numGroups + 1);
    result[numGroups] = '\0';

    int index = len - 1;
    int resIndex = numGroups - 1;

    while (index >= 0) {
        unsigned char value = 0;

        for (int i = 0; i < r; i++) {
            if (index >= 0) {
                value |= (binary[index] - '0') << i;
                index--;
            }
        }

        if (value < 10) {
            result[resIndex--] = '0' + value;
        } else if (value < 32) {
            result[resIndex--] = 'A' + (value - 10);
        }
    }

    return result;
}

void decimalToBinary(int num, char *binary) {
    for (int i = 0; i < 32; i++) {
        binary[i] = (num & (1 << (31 - i))) ? '1' : '0';
    }
    binary[32] = '\0';
}

void printf_convert_2r(int num) {
    char *binary = (char *)malloc(33);
    decimalToBinary(num, binary);
    printf("Двоичное число: %s\n", binary);
    
    for (int r = 1; r <= 5; r++) {
        char *converted = convert_binaryTo_2R(binary, r);
        printf("Основание 2^%d: %s\n", r, converted);
        free(converted);
    }
    free(binary);
}

int main() {
    int number = 12345;
    printf_convert_2r(number);
    
    return 0;
}