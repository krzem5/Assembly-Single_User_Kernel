#ifndef _KERNEL_ACPI_TPM2_H_
#define _KERNEL_ACPI_TPM2_H_ 1
#include <kernel/acpi/structures.h>



extern const acpi_tpm2_t* acpi_tpm2;



void acpi_tpm2_load(const acpi_tpm2_t* tpm2);



#endif
