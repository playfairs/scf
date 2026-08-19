#include "internal.h"

struct scf_format_context {
	scf_format_header header;
	uint32_t initialized;
};

static void scf_format_write_u32(scf_byte *output, uint32_t value)
{
	output[0] = (scf_byte)(value >> 24);
	output[1] = (scf_byte)(value >> 16);
	output[2] = (scf_byte)(value >> 8);
	output[3] = (scf_byte)value;
}

static uint32_t scf_format_read_u32(const scf_byte *input)
{
	return ((uint32_t)input[0] << 24) | ((uint32_t)input[1] << 16) | ((uint32_t)input[2] << 8) | input[3];
}

scf_status scf_format_validate(const scf_format_header *header)
{
	scf_byte empty_magic[sizeof(header->magic)] = {0};

	if (header == NULL || scf_internal_equal(header->magic, empty_magic, sizeof(header->magic)) || header->version == 0 || header->type == 0 || header->payload_size > SIZE_MAX) {
		return SCF_STATUS_FORMAT_INVALID;
	}
	return SCF_STATUS_SUCCESS;
}

scf_status scf_format_context_create(scf_format_context **context)
{
	if (context == NULL) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	*context = scf_internal_alloc(sizeof(**context));
	if (*context == NULL) {
		return SCF_STATUS_ALLOCATION_FAILED;
	}
	scf_internal_clear(*context, sizeof(**context));
	return SCF_STATUS_SUCCESS;
}

scf_status scf_format_header_set(scf_format_context *context, const scf_format_header *header)
{
	if (context == NULL || header == NULL) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	if (scf_format_validate(header) != SCF_STATUS_SUCCESS) {
		return SCF_STATUS_FORMAT_INVALID;
	}
	context->header = *header;
	context->initialized = 1;
	return SCF_STATUS_SUCCESS;
}

scf_status scf_format_header_get(const scf_format_context *context, scf_format_header *header)
{
	if (context == NULL || header == NULL) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	if (!context->initialized) {
		return SCF_STATUS_INVALID_STATE;
	}
	*header = context->header;
	return SCF_STATUS_SUCCESS;
}

scf_status scf_format_serialize(const scf_format_header *header, scf_buffer output, scf_size *written)
{
	uint64_t payload_size;

	if (header == NULL || written == NULL || !scf_internal_output_valid(output)) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	if (scf_format_validate(header) != SCF_STATUS_SUCCESS) {
		return SCF_STATUS_FORMAT_INVALID;
	}
	if (output.size < SCF_FORMAT_HEADER_SIZE) {
		return SCF_STATUS_BUFFER_TOO_SMALL;
	}
	scf_internal_copy(output.data, header->magic, sizeof(header->magic));
	scf_format_write_u32(output.data + 4, header->version);
	scf_format_write_u32(output.data + 8, header->type);
	payload_size = scf_internal_byteswap64(header->payload_size);
	scf_internal_copy(output.data + 12, &payload_size, sizeof(payload_size));
	*written = SCF_FORMAT_HEADER_SIZE;
	return SCF_STATUS_SUCCESS;
}

scf_status scf_format_deserialize(scf_const_buffer input, scf_format_header *header)
{
	uint64_t payload_size;

	if (header == NULL || !scf_internal_buffer_valid(input)) {
		return SCF_STATUS_INVALID_ARGUMENT;
	}
	if (input.size < SCF_FORMAT_HEADER_SIZE) {
		return SCF_STATUS_BUFFER_TOO_SMALL;
	}
	scf_internal_copy(header->magic, input.data, sizeof(header->magic));
	header->version = scf_format_read_u32(input.data + 4);
	header->type = scf_format_read_u32(input.data + 8);
	scf_internal_copy(&payload_size, input.data + 12, sizeof(payload_size));
	header->payload_size = scf_internal_byteswap64(payload_size);
	return scf_format_validate(header);
}

void scf_format_context_destroy(scf_format_context *context)
{
	if (context != NULL) {
		scf_internal_clear(context, sizeof(*context));
		scf_internal_free(context);
	}
}
