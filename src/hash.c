#include "internal.h"

struct scf_hash_context
{
    const scf_hash_provider *provider;
    void *state;
    uint32_t finalized;
};

scf_status
scf_hash_context_create(const scf_hash_provider *provider,
                        scf_hash_context **context)
{
    if (provider == NULL || context == NULL
        || provider->digest_size == 0
        || provider->block_size == 0
        || provider->init == NULL
        || provider->update == NULL
        || provider->final == NULL
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
    (*context)->finalized = 0;
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

scf_status scf_hash_update(scf_hash_context *context,
                           scf_const_buffer input)
{
    if (context == NULL
        || !scf_internal_buffer_valid(input))
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (context->finalized)
    {
        return SCF_STATUS_INVALID_STATE;
    }
    return context->provider->update(context->state, input);
}

scf_status scf_hash_final(scf_hash_context *context,
                          scf_buffer output,
                          scf_size *written)
{
    scf_status status;

    if (context == NULL || written == NULL
        || !scf_internal_output_valid(output))
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (context->finalized)
    {
        return SCF_STATUS_INVALID_STATE;
    }
    if (output.size < context->provider->digest_size)
    {
        return SCF_STATUS_BUFFER_TOO_SMALL;
    }
    status = context->provider->final(context->state,
                                      output,
                                      written);
    if (status == SCF_STATUS_SUCCESS
        && *written > output.size)
    {
        return SCF_STATUS_BUFFER_TOO_SMALL;
    }
    if (status == SCF_STATUS_SUCCESS)
    {
        context->finalized = 1;
    }
    return status;
}

scf_status scf_hash_reset(scf_hash_context *context)
{
    if (context == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (context->provider->reset(context->state)
        != SCF_STATUS_SUCCESS)
    {
        return SCF_STATUS_INVALID_STATE;
    }
    context->finalized = 0;
    return SCF_STATUS_SUCCESS;
}

scf_size
scf_hash_digest_size(const scf_hash_context *context)
{
    return context == NULL ? 0
                           : context->provider->digest_size;
}

scf_size
scf_hash_block_size(const scf_hash_context *context)
{
    return context == NULL ? 0
                           : context->provider->block_size;
}

void scf_hash_context_destroy(scf_hash_context *context)
{
    if (context != NULL)
    {
        context->provider->destroy(context->state);
        scf_internal_clear(context, sizeof(*context));
        scf_internal_free(context);
    }
}
