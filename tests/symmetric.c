// Árbol completo aplanado {6,4,4,1,2,2,1} en 0x1000; a0 = is_symmetric(...)
static int mirror(int *t, int n, int a, int b) {
    if (a >= n && b >= n) return 1;
    if (a >= n || b >= n) return 0;
    return t[a] == t[b] && mirror(t, n, 2*a+1, 2*b+2) && mirror(t, n, 2*a+2, 2*b+1);
}
int is_symmetric(int *t, int n) { return n == 0 || mirror(t, n, 1, 2); }
int main(void) {
    volatile int *tree = (int *)0x1000;
    int init[7] = {6, 4, 4, 1, 2, 2, 1};
    for (int i = 0; i < 7; i++) tree[i] = init[i];
    return is_symmetric((int *)tree, 7);
}
