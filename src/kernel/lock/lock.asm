;;; bit 0 - global lock
;;; bit 1 - multiaccess lock
;;; bit 2 - multiaccess active
;;; bits 3-31 - multiaccess counter



global spinlock_init:function
global spinlock_acquire_exclusive:function
global spinlock_release_exclusive:function
global spinlock_acquire_shared:function
global spinlock_release_shared:function
section .text exec nowrite



[bits 64]
spinlock_init:
	mov dword [rdi], 0
	ret



_spinlock_acquire_exclusive_global_wait:
	pause
	test dword [rdi], 1
	jnz _spinlock_acquire_exclusive_global_wait
spinlock_acquire_exclusive:
	lock bts dword [rdi], 0
	jc _spinlock_acquire_exclusive_global_wait
	ret



spinlock_release_exclusive:
	btr dword [rdi], 0
	ret



_spinlock_acquire_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _spinlock_acquire_shared_multiaccess_wait
spinlock_acquire_shared:
	lock bts dword [rdi], 1
	jc _spinlock_acquire_shared_multiaccess_wait
	test dword [rdi], 4
	jnz ._multiaccess_active
	jmp ._global_test
._global_wait:
	pause
	test dword [rdi], 1
	jnz ._global_wait
._global_test:
	lock bts dword [rdi], 0
	jc ._global_wait
	bts dword [rdi], 2
._multiaccess_active:
	add dword [rdi], 8
	btr dword [rdi], 1
	ret



_spinlock_release_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _spinlock_release_shared_multiaccess_wait
spinlock_release_shared:
	lock bts dword [rdi], 1
	jc _spinlock_release_shared_multiaccess_wait
	sub dword [rdi], 8
	cmp dword [rdi], 8
	jge ._still_used
	mov dword [rdi], 0
	ret
._still_used:
	btr dword [rdi], 1
	ret
