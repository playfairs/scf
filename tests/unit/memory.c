#include <stddef.h>
#include <stdint.h>

#include <scf/memory.h>

int scf_unit_memory(void)
{
	scf_secure_allocation *allocation = NULL;
	scf_buffer buffer;
	scf_size size;

	if (scf_secure_allocate(32, 32, &allocation) != SCF_STATUS_SUCCESS || scf_secure_data(allocation, &buffer) != SCF_STATUS_SUCCESS || ((uintptr_t)buffer.data % 32) != 0 || buffer.size != 32 || scf_secure_size(allocation, &size) != SCF_STATUS_SUCCESS || size != 32) {
		scf_secure_destroy(allocation);
		return 1;
	}
	for (scf_size index = 0; index < buffer.size; ++index) {
		if (buffer.data[index] != 0) {
			scf_secure_destroy(allocation);
			return 1;
		}
		buffer.data[index] = (scf_byte)(index + 1);
	}
	if (scf_secure_clear(allocation) != SCF_STATUS_SUCCESS || scf_secure_clear(allocation) != SCF_STATUS_SUCCESS) {
		scf_secure_destroy(allocation);
		return 1;
	}
	for (scf_size index = 0; index < buffer.size; ++index) {
		if (buffer.data[index] != 0) {
			scf_secure_destroy(allocation);
			return 1;
		}
	}
	scf_secure_destroy(allocation);

	if (scf_secure_allocate(0, 0, &allocation) != SCF_STATUS_SUCCESS || scf_secure_data(allocation, &buffer) != SCF_STATUS_SUCCESS || buffer.data != NULL || buffer.size != 0) {
		scf_secure_destroy(allocation);
		return 1;
	}
	scf_secure_destroy(allocation);

	if (scf_secure_allocate(1, 3, &allocation) != SCF_STATUS_INVALID_ARGUMENT || scf_secure_allocate(SIZE_MAX, 16, &allocation) != SCF_STATUS_OVERFLOW || scf_secure_allocate(1, 0, NULL) != SCF_STATUS_INVALID_ARGUMENT || scf_secure_data(NULL, &buffer) != SCF_STATUS_INVALID_ARGUMENT || scf_secure_size(NULL, &size) != SCF_STATUS_INVALID_ARGUMENT || scf_secure_clear(NULL) != SCF_STATUS_INVALID_ARGUMENT) {
		return 1;
	}
	scf_secure_destroy(NULL);
	return 0;
}