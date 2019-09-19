.text
.global main

main:
		li	$t9, 0xfff3
		beq	$t8, $t9, pt
		li	$t8, 0xfff3
		li	$a0, 0xa0900000
		li	$a1, 0
		li	$a2, 0x200
		lw	$t1,read_sd_card
		jal	$t1
		j	0xa0900000
	# 1) task1 call BIOS print string "It's bootblock!"
pt:
		la	$a0, msg
		lw	$t0, printstr
		jal	$t0
	# 2) task2 call BIOS read kernel in SD card and jump to kernel start
		lw	$a0, kernel
		li	$a1, 512
		lw	$t0, os_size
		lbu	$t3, 0($t0)
        li  $t4, 0x200
        mul $a2, $t3, $t4
		lw	$t1, read_sd_card
		jal	$t1
		j	0xa0800000
# while(1) --> stop here
stop:
	j stop

.data

msg: .ascii "It's a bootloader...\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .word 0x80011000

# 2. PMON print string function address
# printstr(char *string)
printstr: .word 0x80011100

# 3. PMON print char function address
# printch(char ch)
printch: .word 0x80011140

# 4. kernel address (move kernel to here ~)
kernel : .word 0xa0800000

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .word 0xa0800000

os_size : .word 0xa08001fd

/*
.text
.global main

main:
    #0) move bootloader
    li  $t9, 0xffff
    beq $t9, $t1, pt
    lw  $a0, bootloader
    li  $a1, 0x000
    li  $a2, 0x200
    lw  $t0, read_sd_card
    jal $t0
    li  $t1, 0xffff
    j   0xa0806000

pt:
	# 1) task1 call BIOS print string "It's bootblock!"
	la	$a0, msg
	lw	$t0, printstr
	jal	$t0
	# 2) task2 call BIOs read kernel in SD card and jump to kernel start
	lw	$a0, kernel
	li	$a1, 0x200
	lw  $t2, bootblock
	lb  $t3, 0x1fd($t2)
	mul $a2, $t3, 0x200
	lw	$t0, read_sd_card
	jal	$t0
	lw	$t0, kernel_main
	jal	$t0

# while(1) --> stop here
stop:
	j stop

.data

msg: .ascii "It's bootblock!\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .word 0x80011000

# 2. PMON print string function address
# printstr(char *string)
printstr: .word 0x80011100

# 3. PMON print char function address
# printch(char ch)
printch: .word 0x80011140

# 4. kernel address (move kernel to here ~)
kernel : .word 0xa0800000

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .word 0xa0800000

# 6. bootblock address
bootblock : .word 0xa0800000

# 7. a tempt bootblock address for BONUS
bootloader : .word 0xa0806000
*/
