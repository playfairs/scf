#include "internal.h"

struct scf_kdf_context
{
    const scf_kdf_provider *provider;
    void *state;
};

scf_status
scf_kdf_context_create(const scf_kdf_provider *provider,
                       scf_kdf_context **context)
{
    if (provider == NULL || context == NULL
        || provider->init == NULL
        || provider->derive == NULL
        || provider->reset == NULL
        || provider->destroy == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *context = scf_internal_alloc(sizeof(**context));
    if (*context == NULL)
    {
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    (*context)->provider = provider;
    (*context)->state = NULL;
    if (provider->init(&(*context)->state)
        != SCF_STATUS_SUCCESS)
    {
        scf_internal_clear(*context, sizeof(**context));
        scf_internal_free(*context);
        *context = NULL;
        return SCF_STATUS_INVALID_STATE;
    }
    return SCF_STATUS_SUCCESS;
}

scf_status scf_kdf_derive(scf_kdf_context *context,
                          scf_const_buffer password,
                          scf_const_buffer salt,
                          scf_buffer output)
{
    if (context == NULL
        || !scf_internal_buffer_valid(password)
        || !scf_internal_buffer_valid(salt)
        || !scf_internal_output_valid(output))
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (salt.size < context->provider->minimum_salt_size)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (context->provider->maximum_output_size != 0
        && output.size
               > context->provider->maximum_output_size)
    {
        return SCF_STATUS_BUFFER_TOO_SMALL;
    }
    return context->provider->derive(context->state,
                                     password,
                                     salt,
                                     output);
}

scf_status scf_kdf_reset(scf_kdf_context *context)
{
    if (context == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    return context->provider->reset(context->state);
}

void scf_kdf_context_destroy(scf_kdf_context *context)
{
    if (context != NULL)
    {
        context->provider->destroy(context->state);
        scf_internal_clear(context, sizeof(*context));
        scf_internal_free(context);
    }
}
