#include <stdint.h>

#include <scf/scf.h>

int main(void)
{
	const uint64_t input = UINT64_C(41);

	if (scf_test(input) != UINT64_C(42)) {
		return 1;
	}

	if (scf_test(UINT64_MAX) != 0) {
		return 1;
	}

	return 0;
}
