# Generado: una prueba por instrucción. Resultado: a0 = 1 si todas pasan.
        li s0, 0
# prueba 1: add t0, a1, a2
        li s0, 1
        li a1, -8
        li a2, 3
        add t0, a1, a2
        li t1, -5
        bne t0, t1, fail
# prueba 2: sub t0, a1, a2
        li s0, 2
        li a1, 5
        li a2, 9
        sub t0, a1, a2
        li t1, -4
        bne t0, t1, fail
# prueba 3: sll t0, a1, a2
        li s0, 3
        li a1, 1
        li a2, 33
        sll t0, a1, a2
        li t1, 2
        bne t0, t1, fail
# prueba 4: slt t0, a1, a2
        li s0, 4
        li a1, -1
        li a2, 1
        slt t0, a1, a2
        li t1, 1
        bne t0, t1, fail
# prueba 5: sltu t0, a1, a2
        li s0, 5
        li a1, -1
        li a2, 1
        sltu t0, a1, a2
        li t1, 0
        bne t0, t1, fail
# prueba 6: xor t0, a1, a2
        li s0, 6
        li a1, 0xF0F0
        li a2, 0xFF00
        xor t0, a1, a2
        li t1, 4080
        bne t0, t1, fail
# prueba 7: srl t0, a1, a2
        li s0, 7
        li a1, -16
        li a2, 2
        srl t0, a1, a2
        li t1, 1073741820
        bne t0, t1, fail
# prueba 8: sra t0, a1, a2
        li s0, 8
        li a1, -16
        li a2, 2
        sra t0, a1, a2
        li t1, -4
        bne t0, t1, fail
# prueba 9: or t0, a1, a2
        li s0, 9
        li a1, 0xF0
        li a2, 0x0F
        or t0, a1, a2
        li t1, 255
        bne t0, t1, fail
# prueba 10: and t0, a1, a2
        li s0, 10
        li a1, 0xF0
        li a2, 0x3C
        and t0, a1, a2
        li t1, 48
        bne t0, t1, fail
# prueba 11: addi t0, a1, -2048
        li s0, 11
        li a1, 100
        addi t0, a1, -2048
        li t1, -1948
        bne t0, t1, fail
# prueba 12: slli t0, a1, 31
        li s0, 12
        li a1, 3
        slli t0, a1, 31
        li t1, -2147483648
        bne t0, t1, fail
# prueba 13: slti t0, a1, -4
        li s0, 13
        li a1, -5
        slti t0, a1, -4
        li t1, 1
        bne t0, t1, fail
# prueba 14: sltiu t0, a1, -1
        li s0, 14
        li a1, 5
        sltiu t0, a1, -1
        li t1, 1
        bne t0, t1, fail
# prueba 15: xori t0, a1, -1
        li s0, 15
        li a1, 0x55
        xori t0, a1, -1
        li t1, -86
        bne t0, t1, fail
# prueba 16: srli t0, a1, 28
        li s0, 16
        li a1, -1
        srli t0, a1, 28
        li t1, 15
        bne t0, t1, fail
# prueba 17: srai t0, a1, 4
        li s0, 17
        li a1, -256
        srai t0, a1, 4
        li t1, -16
        bne t0, t1, fail
# prueba 18: ori t0, a1, 0x7FF
        li s0, 18
        li a1, 0x100
        ori t0, a1, 0x7FF
        li t1, 2047
        bne t0, t1, fail
# prueba 19: andi t0, a1, 0x123
        li s0, 19
        li a1, -1
        andi t0, a1, 0x123
        li t1, 291
        bne t0, t1, fail
# prueba 20: lui t0, 0xABCDE
        li s0, 20
        lui t0, 0xABCDE
        li t1, -1412571136
        bne t0, t1, fail
# prueba 21: 1: auipc t0, 1; la t3, 1b; sub t0, t0, t3
        li s0, 21
        1: auipc t0, 1
        la t3, 1b
        sub t0, t0, t3
        li t1, 4096
        bne t0, t1, fail
# prueba 22: sb a2, 3(a1); lb t0, 3(a1)
        li s0, 22
        li a1, 0x2000
        li a2, 0x80FF
        sb a2, 3(a1)
        lb t0, 3(a1)
        li t1, -1
        bne t0, t1, fail
# prueba 23: sh a2, 6(a1); lh t0, 6(a1)
        li s0, 23
        li a1, 0x2000
        li a2, 0x8001
        sh a2, 6(a1)
        lh t0, 6(a1)
        li t1, -32767
        bne t0, t1, fail
# prueba 24: sw a2, 8(a1); lw t0, 8(a1)
        li s0, 24
        li a1, 0x2000
        li a2, 0xDEADBEEF
        sw a2, 8(a1)
        lw t0, 8(a1)
        li t1, -559038737
        bne t0, t1, fail
# prueba 25: sb a2, 12(a1); lbu t0, 12(a1)
        li s0, 25
        li a1, 0x2000
        li a2, 0x80
        sb a2, 12(a1)
        lbu t0, 12(a1)
        li t1, 128
        bne t0, t1, fail
# prueba 26: sh a2, 14(a1); lhu t0, 14(a1)
        li s0, 26
        li a1, 0x2000
        li a2, 0xFFFF
        sh a2, 14(a1)
        lhu t0, 14(a1)
        li t1, 65535
        bne t0, t1, fail
# prueba 27: sw a2, 0(a1); lbu t0, 0(a1)
        li s0, 27
        li a1, 0x2010
        li a2, 0x11223344
        sw a2, 0(a1)
        lbu t0, 0(a1)
        li t1, 68
        bne t0, t1, fail
# prueba 28: lhu t0, 2(a1)
        li s0, 28
        li a1, 0x2010
        lhu t0, 2(a1)
        li t1, 4386
        bne t0, t1, fail
# prueba 29: sw a1, -4(a1); lw t0, -4(a1)
        li s0, 29
        li a1, 0x2020
        sw a1, -4(a1)
        lw t0, -4(a1)
        li t1, 8224
        bne t0, t1, fail
# prueba 30: beq a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 30
        li a1, 7
        li a2, 7
        li t0, 0
        beq a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 1
        bne t0, t1, fail
# prueba 31: bne a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 31
        li a1, 7
        li a2, 8
        li t0, 0
        bne a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 1
        bne t0, t1, fail
# prueba 32: blt a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 32
        li a1, -1
        li a2, 1
        li t0, 0
        blt a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 1
        bne t0, t1, fail
# prueba 33: bge a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 33
        li a1, 1
        li a2, -1
        li t0, 0
        bge a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 1
        bne t0, t1, fail
# prueba 34: bltu a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 34
        li a1, 1
        li a2, -1
        li t0, 0
        bltu a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 1
        bne t0, t1, fail
# prueba 35: bgeu a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 35
        li a1, -1
        li a2, 1
        li t0, 0
        bgeu a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 1
        bne t0, t1, fail
# prueba 36: blt a1, a2, 1f; li t0, 9; 1: addi t0, t0, 1
        li s0, 36
        li a1, 1
        li a2, -1
        li t0, 0
        blt a1, a2, 1f
        li t0, 9
        1: addi t0, t0, 1
        li t1, 10
        bne t0, t1, fail
# prueba 37: jal t2, 1f; li t0, 9; 1: la t3, 1b; sub t0, t3, t2
        li s0, 37
        li t0, 0
        jal t2, 1f
        li t0, 9
        1: la t3, 1b
        sub t0, t3, t2
        li t1, 4
        bne t0, t1, fail
# prueba 38: la t3, 1f; jalr t2, 0(t3); li t0, 9; 1: addi t0, t0, 3
        li s0, 38
        li t0, 0
        la t3, 1f
        jalr t2, 0(t3)
        li t0, 9
        1: addi t0, t0, 3
        li t1, 3
        bne t0, t1, fail
# prueba 39: jalr t3, 0(t3); 1: la t4, 1b; sub t0, t3, t4
        li s0, 39
        la t3, 1f
        addi t3, t3, 1
        jalr t3, 0(t3)
        1: la t4, 1b
        sub t0, t3, t4
        li t1, 0
        bne t0, t1, fail
# prueba 40: addi t0, zero, 0
        li s0, 40
        li zero, 55
        addi t0, zero, 0
        li t1, 0
        bne t0, t1, fail
        li a0, 1
        j done
fail:   li a0, 0
done:   j done
