#define DECLARE_SECTION_START(name) __KERNEL_SECTION_##name##_START__ = .
#define DECLARE_SECTION_END(name) __KERNEL_SECTION_##name##_END__ = .

#define BASE_ADDRESS_OFFSET 0xffffffffc0000000



ENTRY(_start)
SECTIONS{
	. = BASE_ADDRESS_OFFSET + 0x100000;
	DECLARE_SECTION_START(kernel);
	DECLARE_SECTION_START(kernel_ue);
	.kernel_ue : AT(ADDR(.kernel_ue) - BASE_ADDRESS_OFFSET){
		*(.entry)
		*(.etext .etext*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_ue);
	DECLARE_SECTION_START(kernel_ur);
	.kernel_ur : AT(ADDR(.kernel_ur) - BASE_ADDRESS_OFFSET){
		*(.erdata .erdata*)
		. = ALIGN(8);
		DECLARE_SECTION_START(cpu_local);
		*(.cpulocal)
		. = ALIGN(8);
		DECLARE_SECTION_END(cpu_local);
		DECLARE_SECTION_START(early_early_initializers);
		*(.eeinitializer)
		. = ALIGN(8);
		DECLARE_SECTION_END(early_early_initializers);
		DECLARE_SECTION_START(early_initializers);
		*(.einitializer)
		. = ALIGN(8);
		DECLARE_SECTION_END(early_initializers);
		DECLARE_SECTION_START(initializers);
		*(.initializer)
		. = ALIGN(8);
		DECLARE_SECTION_END(initializers);
		. = ALIGN(0x1000);
		*(.ap_startup)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_ur);
	DECLARE_SECTION_START(kernel_uw);
	.kernel_uw : AT(ADDR(.kernel_uw) - BASE_ADDRESS_OFFSET){
		*(.edata .edata*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_uw);
	DECLARE_SECTION_START(kernel_ex);
	.kernel_ex : AT(ADDR(.kernel_ex) - BASE_ADDRESS_OFFSET){
		*(.text .text*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_ex);
	DECLARE_SECTION_START(kernel_nx);
	.kernel_nx : AT(ADDR(.kernel_nx) - BASE_ADDRESS_OFFSET){
		*(.rodata .rodata* .rdata .rdata*)
		. = ALIGN(8);
		DECLARE_SECTION_START(gcov_info);
#ifdef _KERNEL_COVERAGE_BUILD_
		*(.gcov_info)
#endif
		. = ALIGN(8);
		DECLARE_SECTION_END(gcov_info);
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_nx);
	DECLARE_SECTION_START(kernel_rw);
	.kernel_rw : AT(ADDR(.kernel_rw) - BASE_ADDRESS_OFFSET){
		*(.data .data*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_rw);
	DECLARE_SECTION_START(kernel_iw);
	.kernel_iw : AT(ADDR(.kernel_iw) - BASE_ADDRESS_OFFSET){
		*(.idata .idata*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_iw);
	DECLARE_SECTION_START(kernel_zw);
	.kernel_zw : AT(ADDR(.kernel_zw) - BASE_ADDRESS_OFFSET){
		*(.bss)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_zw);
	DECLARE_SECTION_END(kernel);
#ifndef _KERNEL_DEBUG_BUILD_
	/DISCARD/ : {
		*(*)
	}
#endif
}
