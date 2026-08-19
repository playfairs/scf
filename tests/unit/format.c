#include <string.h>

#include <scf/format.h>

int scf_unit_format(void)
{
    scf_format_header input = {{'S', 'C', 'F', 1}, 1, 1, 37};
    scf_format_header output;
    scf_byte serialized[SCF_FORMAT_HEADER_SIZE] = {0};
    scf_size written = 0;

    if (scf_format_serialize(&input, (scf_buffer){serialized, sizeof(serialized)}, &written) != SCF_STATUS_SUCCESS || written != sizeof(serialized) || scf_format_deserialize((scf_const_buffer){serialized, sizeof(serialized)}, &output) != SCF_STATUS_SUCCESS || memcmp(input.magic, output.magic, sizeof(input.magic)) != 0 || input.version != output.version || input.type != output.type || input.payload_size != output.payload_size) {
        return 1;
    }
    return scf_format_validate(&(scf_format_header){{0, 0, 0, 0}, 1, 1, 0}) == SCF_STATUS_FORMAT_INVALID ? 0 : 1;
}
