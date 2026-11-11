#include <stddef.h>

int main() {
    int *p = NULL;
    *p = 42; // Вызовет Segmentation Fault
    return 0;
}
