#include <stdio.h>

#include <scf/scf.h>

int scf_unit_scf(void);
int scf_unit_hash(void);
int scf_unit_cipher(void);
int scf_unit_kdf(void);
int scf_unit_keys(void);
int scf_unit_format(void);
int scf_unit_provider(void);
int scf_vector_framework(void);

int main(void)
{
    const struct {
        const char *name;
        int (*run)(void);
    } tests[] = {
        {"scf", scf_unit_scf},
        {"hash", scf_unit_hash},
        {"cipher", scf_unit_cipher},
        {"kdf", scf_unit_kdf},
        {"keys", scf_unit_keys},
        {"format", scf_unit_format},
        {"provider", scf_unit_provider},
        {"vectors", scf_vector_framework}
    };

    for (scf_size index = 0; index < sizeof(tests) / sizeof(tests[0]); ++index) {
        if (tests[index].run() != 0) {
            fprintf(stderr, "%s test failed\n", tests[index].name);
            return 1;
        }
    }
    puts("scf tests passed");
    return 0;
}
