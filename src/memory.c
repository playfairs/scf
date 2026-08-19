#include <stdint.h>

#include "internal.h"

struct scf_secure_allocation
{
    scf_byte *data;
    scf_byte *raw;
    scf_size size;
    scf_size alignment;
};

static int scf_secure_alignment_valid(scf_size alignment)
{
    return alignment >= sizeof(void *)
           && (alignment & (alignment - 1)) == 0;
}

static int scf_secure_add_overflow(scf_size left,
                                   scf_size right,
                                   scf_size *result)
{
    if (right > SIZE_MAX - left)
    {
        return 1;
    }
    *result = left + right;
    return 0;
}

static int scf_secure_align_up(scf_size value,
                               scf_size alignment,
                               scf_size *result)
{
    scf_size remainder = value & (alignment - 1);

    if (remainder == 0)
    {
        *result = value;
        return 0;
    }
    return scf_secure_add_overflow(value,
                                   alignment - remainder,
                                   result);
}

scf_status
scf_secure_allocate(scf_size size,
                    scf_size alignment,
                    scf_secure_allocation **allocation)
{
    scf_secure_allocation *result;
    scf_size raw_size;
    scf_size aligned_offset;

    if (allocation == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *allocation = NULL;
    if (alignment != 0
        && !scf_secure_alignment_valid(alignment))
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    result = scf_internal_alloc(sizeof(*result));
    if (result == NULL)
    {
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    result->data = NULL;
    result->raw = NULL;
    result->size = size;
    result->alignment =
        alignment == 0 ? sizeof(void *) : alignment;
    if (size == 0)
    {
        *allocation = result;
        return SCF_STATUS_SUCCESS;
    }
    if (scf_secure_add_overflow(size,
                                result->alignment - 1,
                                &raw_size)
        || scf_secure_add_overflow(raw_size,
                                   sizeof(void *),
                                   &raw_size))
    {
        scf_internal_clear(result, sizeof(*result));
        scf_internal_free(result);
        return SCF_STATUS_OVERFLOW;
    }
    result->raw = scf_internal_alloc(raw_size);
    if (result->raw == NULL)
    {
        scf_internal_clear(result, sizeof(*result));
        scf_internal_free(result);
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    if (scf_secure_align_up(
            (scf_size)(uintptr_t)(result->raw
                                  + sizeof(void *)),
            result->alignment,
            &aligned_offset))
    {
        scf_internal_free(result->raw);
        scf_internal_clear(result, sizeof(*result));
        scf_internal_free(result);
        return SCF_STATUS_OVERFLOW;
    }
    result->data = (scf_byte *)(uintptr_t)aligned_offset;
    scf_internal_clear(result->data, size);
    *allocation = result;
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_secure_data(scf_secure_allocation *allocation,
                scf_buffer *buffer)
{
    if (allocation == NULL || buffer == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    buffer->data = allocation->data;
    buffer->size = allocation->size;
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_secure_size(const scf_secure_allocation *allocation,
                scf_size *size)
{
    if (allocation == NULL || size == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *size = allocation->size;
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_secure_clear(scf_secure_allocation *allocation)
{
    if (allocation == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_internal_clear(allocation->data, allocation->size);
    return SCF_STATUS_SUCCESS;
}

void scf_secure_destroy(scf_secure_allocation *allocation)
{
    if (allocation != NULL)
    {
        scf_internal_clear(allocation->data,
                           allocation->size);
        scf_internal_free(allocation->raw);
        scf_internal_clear(allocation, sizeof(*allocation));
        scf_internal_free(allocation);
    }
}