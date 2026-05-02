	.text
	.file	"dsl_program"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$32, %rsp
	movl	$0, -4(%rbp)
	movl	$10, -8(%rbp)
	movl	$20, -12(%rbp)
	movl	-8(%rbp), %eax
	addl	-12(%rbp), %eax
	movl	%eax, -16(%rbp)
	movl	-16(%rbp), %esi
	leaq	.fmt_int(%rip), %rdi
	movl	$0, %eax
	callq	printf
	movl	-16(%rbp), %eax
	cmpl	$25, %eax
	jle	.LBB0_2
.LBB0_1:
	leaq	.str0(%rip), %rsi
	leaq	.fmt_str(%rip), %rdi
	movl	$0, %eax
	callq	printf
	jmp	.LBB0_3
.LBB0_2:
	leaq	.str1(%rip), %rsi
	leaq	.fmt_str(%rip), %rdi
	movl	$0, %eax
	callq	printf
.LBB0_3:
	movl	$0, -20(%rbp)
.LBB0_4:
	cmpl	$5, -20(%rbp)
	jge	.LBB0_6
	movl	-20(%rbp), %esi
	leaq	.fmt_int(%rip), %rdi
	movl	$0, %eax
	callq	printf
	addl	$1, -20(%rbp)
	jmp	.LBB0_4
.LBB0_6:
	xorl	%eax, %eax
	addq	$32, %rsp
	popq	%rbp
	retq
.fmt_int:
	.asciz	"%d\n"
.fmt_str:
	.asciz	"%s\n"
.str0:
	.asciz	"Greater than 25"
.str1:
	.asciz	"Not greater"
