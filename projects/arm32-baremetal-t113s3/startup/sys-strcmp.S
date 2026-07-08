	.text

    .global sys_strcmp
    .type sys_strcmp, %function
    .align 4

sys_strcmp:
1:	ldrb	r2, [r0], #1
	ldrb	r3, [r1], #1
	cmp	r2, #1
	cmpcs	r2, r3
	beq	1b
	sub	r0, r2, r3
	mov	pc, lr
