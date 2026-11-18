int main(void) { volatile int *p = (volatile int *)0; *p = 42; return 0; }
