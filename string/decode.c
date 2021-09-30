#include <string.h>

void decodeString(char *str, int length, unsigned char key) {
    for (int i = 0; i < length; i++) {
        str[i] ^= key;
    }
}
