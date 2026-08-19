#include <scf/kdf.h>

static scf_status kdf_init(void **state)
{
    *state = (void *)1;
    return SCF_STATUS_SUCCESS;
}

static scf_status kdf_derive(void *state,
                             scf_const_buffer password,
                             scf_const_buffer salt,
                             scf_buffer output)
{
    (void)state;
    for (scf_size index = 0; index < output.size; ++index)
    {
        output.data[index] =
            (scf_byte)(password.size + salt.size + index);
    }
    return SCF_STATUS_SUCCESS;
}

static scf_status kdf_reset(void *state)
{
    (void)state;
    return SCF_STATUS_SUCCESS;
}

static void kdf_destroy(void *state)
{
    (void)state;
}

int scf_unit_kdf(void)
{
    scf_kdf_provider provider = {"test",
                                 2,
                                 16,
                                 kdf_init,
                                 kdf_derive,
                                 kdf_reset,
                                 kdf_destroy};
    scf_kdf_context *context = NULL;
    scf_byte password[3] = {1, 2, 3};
    scf_byte salt[2] = {4, 5};
    scf_byte output[4] = {0};

    if (scf_kdf_context_create(&provider, &context)
            != SCF_STATUS_SUCCESS
        || scf_kdf_derive(
               context,
               (scf_const_buffer){password,
                                  sizeof(password)},
               (scf_const_buffer){salt, sizeof(salt)},
               (scf_buffer){output, sizeof(output)})
               != SCF_STATUS_SUCCESS
        || output[0] != 5 || output[3] != 8
        || scf_kdf_reset(context) != SCF_STATUS_SUCCESS)
    {
        scf_kdf_context_destroy(context);
        return 1;
    }
    scf_kdf_context_destroy(context);
    return 0;
}
