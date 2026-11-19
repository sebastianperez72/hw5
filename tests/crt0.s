# Arranque mínimo: llama a main y deja el resultado en a0; luego bucle infinito.
.globl _start
_start: call main
done:   j done
