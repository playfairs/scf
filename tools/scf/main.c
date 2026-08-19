#include <stdio.h>
#include <string.h>

#include <scf/scf.h>

static int scf_tool_self_test(void)
{
    scf_status status = scf_self_test();
    if (status != SCF_STATUS_SUCCESS)
    {
        fprintf(stderr,
                "self-test failed: %s\n",
                scf_status_name(status));
        return 1;
    }
    puts("self-test passed");
    return 0;
}

int main(int argc, char **argv)
{
    scf_version_info version;
    scf_runtime_info runtime;

    if (scf_version(&version) != SCF_STATUS_SUCCESS
        || scf_runtime(&runtime) != SCF_STATUS_SUCCESS)
    {
        return 1;
    }
    if (argc == 1 || strcmp(argv[1], "version") == 0)
    {
        printf("SCF %s\n", version.text);
        return 0;
    }
    if (strcmp(argv[1], "architecture") == 0)
    {
        printf("%s %zu-bit\n",
               runtime.architecture,
               runtime.word_size * 8);
        return 0;
    }
    if (strcmp(argv[1], "self-test") == 0)
    {
        return scf_tool_self_test();
    }
    fprintf(stderr, "unknown command: %s\n", argv[1]);
    return 2;
}
