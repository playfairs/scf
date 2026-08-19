#include <string.h>

#include <scf/cipher.h>

static scf_status cipher_init(void **state,
                              scf_const_buffer key,
                              scf_const_buffer nonce)
{
    (void)key;
    (void)nonce;
    *state = (void *)1;
    return SCF_STATUS_SUCCESS;
}

static scf_status cipher_crypt(void *state,
                               scf_const_buffer input,
                               scf_buffer output)
{
    (void)state;
    memcpy(output.data, input.data, input.size);
    return SCF_STATUS_SUCCESS;
}

static scf_status cipher_reset(void *state)
{
    (void)state;
    return SCF_STATUS_SUCCESS;
}

static void cipher_destroy(void *state)
{
    (void)state;
}

int scf_unit_cipher(void)
{
    scf_cipher_provider provider = {"test",
                                    4,
                                    2,
                                    cipher_init,
                                    cipher_crypt,
                                    cipher_crypt,
                                    cipher_reset,
                                    cipher_destroy};
    scf_cipher_context *context = NULL;
    scf_byte key[4] = {1, 2, 3, 4};
    scf_byte nonce[2] = {5, 6};
    scf_byte input[3] = {7, 8, 9};
    scf_byte output[3] = {0};

    if (scf_cipher_context_create(
            &provider,
            (scf_const_buffer){key, sizeof(key)},
            (scf_const_buffer){nonce, sizeof(nonce)},
            &context)
            != SCF_STATUS_SUCCESS
        || scf_cipher_encrypt(
               context,
               (scf_const_buffer){input, sizeof(input)},
               (scf_buffer){output, sizeof(output)})
               != SCF_STATUS_SUCCESS
        || memcmp(input, output, sizeof(input)) != 0
        || scf_cipher_decrypt(
               context,
               (scf_const_buffer){input, sizeof(input)},
               (scf_buffer){output, sizeof(output)})
               != SCF_STATUS_SUCCESS
        || scf_cipher_reset(context) != SCF_STATUS_SUCCESS)
    {
        scf_cipher_context_destroy(context);
        return 1;
    }
    scf_cipher_context_destroy(context);
    return 0;
}
