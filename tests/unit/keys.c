#include <string.h>

#include <scf/keys.h>

int scf_unit_keys(void)
{
    scf_byte material[4] = {1, 2, 3, 4};
    scf_const_buffer data;
    scf_key *key = NULL;
    scf_key *copy = NULL;

    if (scf_key_create(
            (scf_const_buffer){material, sizeof(material)},
            &key)
            != SCF_STATUS_SUCCESS
        || scf_key_copy(key, &copy) != SCF_STATUS_SUCCESS
        || scf_key_size(copy) != sizeof(material)
        || scf_key_state_get(copy) != SCF_KEY_INITIALIZED
        || scf_key_data(copy, &data) != SCF_STATUS_SUCCESS
        || memcmp(data.data, material, sizeof(material))
               != 0
        || scf_key_clear(key) != SCF_STATUS_SUCCESS
        || scf_key_state_get(key) != SCF_KEY_CLEARED)
    {
        scf_key_destroy(key);
        scf_key_destroy(copy);
        return 1;
    }
    scf_key_destroy(key);
    scf_key_destroy(copy);
    return 0;
}
