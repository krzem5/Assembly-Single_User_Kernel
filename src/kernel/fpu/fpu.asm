global fpu_enable:function hidden
section .etext exec nowrite



[bits 64]
fpu_enable:
	push rbx
	mov rax, cr0
	and rax, 0xfffffffffffffffb
	or rax, 0x00000002
	mov cr0, rax
	mov rax, cr4
	or rax, 0x00040600
	mov cr4, rax
	mov eax, 1
	cpuid
	shr ecx, 26
	and ecx, 4
	mov ebx, ecx
	xor ecx, ecx
	xgetbv
	or eax, 0x00000003
	or eax, ebx
	xsetbv
	mov eax, 13
	cpuid
	add ecx, 63
	and ecx, 0xffffffc0
	cmp ecx, dword [fpu_state_size]
	jl ._skip_fpu_state_size_update
	mov dword [fpu_state_size], ecx
._skip_fpu_state_size_update:
	pop rbx
	ret



global fpu_state_size:data hidden 4
global fpu_init:function hidden
global fpu_save:function hidden
global fpu_restore:function hidden
section .text exec nowrite



[bits 64]
fpu_init:
	push rdi
	xor eax, eax
	mov ecx, dword [fpu_state_size]
	shr ecx, 3
	rep stosq
	mov rdi, qword [rsp]
	vzeroall
	mov dword [rsp], 0x037f
	fldcw [rsp]
	mov dword [rsp], 0x1f80
	ldmxcsr [rsp]
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xsave [rdi]
	add rsp, 8
	ret



fpu_save:
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xsave [rdi]
	ret



fpu_restore:
	xor eax, eax
	sub eax, 1
	mov edx, eax
	xrstor [rdi]
	ret



section .idata noexec write



fpu_state_size:
	dd 0
