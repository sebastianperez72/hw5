# Prueba de ecall: imprime texto, un entero, un char y sale con código 7.
        la   a0, msg
        li   a7, 4
        ecall
        li   a0, -42
        li   a7, 1
        ecall
        li   a0, 10
        li   a7, 11
        ecall
        li   a0, 7
        li   a7, 17
        ecall
msg:    .asciz "Hola desde RISC-V! n = "
