#ifndef SCF_H
#define SCF_H

#include <stddef.h>
#include <stdint.h>

#include <scf/version.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define SCF_API __declspec(dllexport)
#else
#define SCF_API __attribute__((visibility("default")))
#endif

typedef uint8_t scf_byte;
typedef size_t scf_size;

typedef struct {
	scf_byte *data;
	scf_size size;
} scf_buffer;

typedef struct {
	const scf_byte *data;
	scf_size size;
} scf_const_buffer;

typedef enum {
	SCF_STATUS_SUCCESS = 0,
	SCF_STATUS_INVALID_ARGUMENT,
	SCF_STATUS_INVALID_STATE,
	SCF_STATUS_BUFFER_TOO_SMALL,
	SCF_STATUS_ALLOCATION_FAILED,
	SCF_STATUS_UNSUPPORTED,
	SCF_STATUS_OVERFLOW,
	SCF_STATUS_FORMAT_INVALID,
	SCF_STATUS_DUPLICATE,
	SCF_STATUS_NOT_FOUND,
	SCF_STATUS_BUSY,
	SCF_STATUS_NAME_TOO_LONG
} scf_status;

typedef struct {
	uint32_t major;
	uint32_t minor;
	uint32_t patch;
	const char *text;
} scf_version_info;

typedef struct {
	char architecture[16];
	scf_size word_size;
	uint32_t assembly_available;
} scf_runtime_info;

typedef struct scf_context scf_context;

SCF_API scf_status scf_init(void);
SCF_API scf_status scf_version(scf_version_info *version);
SCF_API scf_status scf_runtime(scf_runtime_info *runtime);
SCF_API const char *scf_status_name(scf_status status);
SCF_API scf_status scf_context_create(scf_context **context);
SCF_API scf_status scf_context_reset(scf_context *context);
SCF_API void scf_context_destroy(scf_context *context);
SCF_API scf_status scf_self_test(void);
SCF_API uint64_t scf_test(uint64_t value);

#ifdef __cplusplus
}
#endif

#endif