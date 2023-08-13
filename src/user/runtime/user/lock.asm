global lock_init
global lock_acquire_exclusive
global lock_release_exclusive
global lock_acquire_shared
global lock_release_shared
global lock_exclusive_to_shared
global lock_shared_to_exclusive



[bits 64]
section .text.lock_init
lock_init:
	mov dword [rdi], 0
	ret



section .text.lock_acquire_exclusive
_lock_acquire_exclusive_global_wait:
	pause
	test dword [rdi], 1
	jnz _lock_acquire_exclusive_global_wait
lock_acquire_exclusive:
	lock bts dword [rdi], 0
	jc _lock_acquire_exclusive_global_wait
	ret



section .text.lock_release_exclusive
lock_release_exclusive:
	btr dword [rdi], 0
	ret



section .text.lock_acquire_shared
_lock_acquire_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _lock_acquire_shared_multiaccess_wait
lock_acquire_shared:
	lock bts dword [rdi], 1
	jc _lock_acquire_shared_multiaccess_wait
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



section .text.lock_release_shared
_lock_release_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _lock_release_shared_multiaccess_wait
lock_release_shared:
	lock bts dword [rdi], 1
	jc _lock_release_shared_multiaccess_wait
	sub dword [rdi], 8
	cmp dword [rdi], 8
	jge ._still_used
	mov dword [rdi], 0
	ret
._still_used:
	btr dword [rdi], 1
	ret



section .text.lock_exclusive_to_shared
_lock_exclusive_to_shared_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _lock_exclusive_to_shared_multiaccess_wait
lock_exclusive_to_shared:
	lock bts dword [rdi], 1
	jc _lock_exclusive_to_shared_multiaccess_wait
	mov dword [rdi], 0b1101
	ret



section .text.lock_shared_to_exclusive
_lock_shared_to_exclusive_multiaccess_wait:
	pause
	test dword [rdi], 2
	jnz _lock_shared_to_exclusive_multiaccess_wait
lock_shared_to_exclusive:
	lock bts dword [rdi], 1
	jc _lock_shared_to_exclusive_multiaccess_wait
	sub dword [rdi], 8
	cmp dword [rdi], 8
	jge ._still_used
	mov dword [rdi], 1
	ret
._still_used:
	btr dword [rdi], 1
	jmp lock_acquire_exclusive
