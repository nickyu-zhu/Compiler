.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
j main
read:
    li $v0,4
    la $a0,_prompt
    syscall
    li $v0,5
    syscall
    jr $ra

write:
    li $v0,1
    syscall
    li $v0,4
    la $a0,_ret
    syscall
    move $v0,$0
    jr $ra
fibonacci:
	lw $t2,28($sp)	
	lw $t3,24($sp)	
	add $t1,$t2,$t3
	move $t4,$t1
	move $v0,$t4
	jr $ra
label1:
main:
	li $t1,1
	li $t5,1
	addi $sp,$sp,-4
	sw $ra,0($sp)
	jal read
	lw $ra,0($sp)
	move $t6,$v0
	addi $sp,$sp,4
	move $t7,$t6
	move $t7,$t7
	bgt $t7,0,label7
	j label6
label8:
	subi $t7,$t7,1
label7:
	addi $sp,$sp,-4
	sw $t1,0($sp)
	move $a0,$t1
	addi $sp,$sp,-4
	sw $t5,0($sp)
	move $a0,$t5
	addi $sp,$sp,-24
	sw $t0,0($sp)
	sw $ra,4($sp)
	sw $t1,8($sp)
	sw $t2,12($sp)
	sw $t3,16($sp)
	sw $t4,20($sp)
	jal fibonacci
	lw $a0,0($sp)
	lw $ra,4($sp)
	lw $t1,8($sp)
	lw $t2,12($sp)
	lw $t3,16($sp)
	lw $t4,20($sp)
	addi $sp,$sp,24
	addi $sp,$sp,8
	move $t6,$v0
	move $t1,$t6
	addi $sp,$sp,-4
	sw $t1,0($sp)
	move $a0,$t1
	addi $sp,$sp,-4
	sw $ra,0($sp)
	jal write
	lw $ra,0($sp)
	addi $sp,$sp,4
	sub $t6,$t1,$t5
	move $t5,$t6
	j label8
label6:
	li $v0, 10
	syscall
label2:
