# SCF

SCF is the SCARLETT Cryptographic Framework foundation.

The initial foundation contains a public C API, version definitions, architecture-specific Assembly selection, a static library, and a C test executable.

No cryptographic algorithms are implemented.

## Provider mодель (Model)

Algorithm implementations register a descriptor with an explicitly created `scf_provider_registry`. Each descriptor identifies its provider type, numeric identifier, name, capabilities, and context lifecycle callbacks.

Registration copies the descriptor metadata and provider name into the registry. The callback functions and callback-owned implementation remain owned by the registering implementation and must remain valid until the provider is unregistered and all contexts created from it are destroyed.

Lookup returns copied metadata. Context creation selects a registered provider by type and identifier, retains that provider for the lifetime of the context, and invokes its context creation callback. Unregistration prevents new lookups and contexts while allowing existing contexts to finish. Registry destruction returns a busy status while any registered or retained provider remains.

The registry is an explicit opaque object. No mutable provider registry is exposed as global public state. Registry access is synchronized internally, while provider implementations remain responsible for their own callback state.

## Secure Memory

The public `scf_secure_allocate` API returns an opaque allocation handle. The handle owns its storage and must be released with `scf_secure_destroy`. Data returned by `scf_secure_data` is borrowed, remains valid only while the handle is alive, and must not be used after destruction.

Secure allocations are zero-initialized, can be explicitly cleared repeatedly, and are cleared again during destruction through the architecture-specific Assembly primitive. Zero-length allocations are valid and contain no data. Requested alignments must be powers of two and at least pointer alignment. Invalid alignments and overflowing size calculations return an error without allocating storage.

Keys store their material in secure allocations. Provider context records use secure allocations for framework-owned state, while provider callbacks remain responsible for clearing and releasing callback-owned state.

## Self-Tests and KATs

The KAT registry is created explicitly with a provider registry. A KAT descriptor identifies the provider type, provider identifier, test identifier, test name, generic vector buffers, and an execution callback. Vector buffers may contain input, expected output, key, nonce, salt, or auxiliary algorithm-specific data without defining an algorithm-specific public structure.

Registration validates the descriptor, verifies that the provider exists, copies the test name, and copies every vector buffer into secure memory. The caller retains ownership of its original descriptor and buffers. The registry owns its copied data and clears it when a test is unregistered or the registry is destroyed.

Individual tests, all tests for a provider, and all registered tests can be executed deterministically. Results expose provider and test identifiers, bounded names, and a status category only; vector data and key material are never included in results or failure messages. Registered tests are never silently skipped. Missing providers, invalid descriptors, duplicate tests, unsupported callbacks, failed callbacks, internal failures, and empty test sets have distinct statuses.

Provider callbacks must not retain vector pointers after returning. The callback may return success, test failure, unsupported, invalid test, or internal failure. Other callback statuses are normalized to an internal failure so the KAT boundary remains predictable for future startup or FIPS-style validation.

No cryptographic algorithm is included in the KAT framework. The current dummy provider and vectors exist only to exercise registration, dispatch, cleanup, and failure handling.
