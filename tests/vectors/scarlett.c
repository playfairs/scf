#include <scf/scf.h>

int scf_vector_framework(void)
{
    static const uint64_t inputs[] = {0,
                                      1,
                                      41,
                                      UINT64_MAX - 1};
    static const uint64_t outputs[] = {1,
                                       2,
                                       42,
                                       UINT64_MAX};

    for (scf_size index = 0;
         index < sizeof(inputs) / sizeof(inputs[0]);
         ++index)
    {
        if (scf_test(inputs[index]) != outputs[index])
        {
            return 1;
        }
    }
    return 0;
}
