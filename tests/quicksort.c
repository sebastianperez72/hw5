// Coloca {6,4,3,2,1,8,9} en 0x1000 y lo ordena in-place con quicksort recursivo.
static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }
static int partition(int *v, int lo, int hi) {
    int pivot = v[hi], i = lo - 1;
    for (int j = lo; j < hi; j++)
        if (v[j] < pivot) swap(&v[++i], &v[j]);
    swap(&v[i + 1], &v[hi]);
    return i + 1;
}
static void quicksort(int *v, int lo, int hi) {
    if (lo < hi) {
        int p = partition(v, lo, hi);
        quicksort(v, lo, p - 1);
        quicksort(v, p + 1, hi);
    }
}
int main(void) {
    volatile int *arr = (int *)0x1000;
    int init[7] = {6, 4, 3, 2, 1, 8, 9};
    for (int i = 0; i < 7; i++) arr[i] = init[i];
    quicksort((int *)arr, 0, 6);
    return 0;
}
