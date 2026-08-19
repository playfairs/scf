#include <scf/scf.h>

int scf_unit_scf(void)
{
    scf_context *context = NULL;
    scf_version_info version;
    scf_runtime_info runtime;

    if (scf_init() != SCF_STATUS_SUCCESS || scf_version(&version) != SCF_STATUS_SUCCESS || version.major != SCF_VERSION_MAJOR || scf_runtime(&runtime) != SCF_STATUS_SUCCESS || runtime.assembly_available == 0 || scf_context_create(&context) != SCF_STATUS_SUCCESS || scf_context_reset(context) != SCF_STATUS_SUCCESS) {
        scf_context_destroy(context);
        return 1;
    }
    scf_context_destroy(context);
    return scf_status_name(SCF_STATUS_SUCCESS)[0] == 's' ? 0 : 1;
}
