# SCF

SCF is the SCARLETT Cryptographic Framework foundation.

The initial foundation contains a public C API, version definitions, architecture-specific Assembly selection, a static library, and a C test executable.

No cryptographic algorithms are implemented.

## Provider mодель (Model)

Algorithm implementations register a descriptor with an explicitly created `scf_provider_registry`. Each descriptor identifies its provider type, numeric identifier, name, capabilities, and context lifecycle callbacks.

Registration copies the descriptor metadata and provider name into the registry. The callback functions and callback-owned implementation remain owned by the registering implementation and must remain valid until the provider is unregistered and all contexts created from it are destroyed.

Lookup returns copied metadata. Context creation selects a registered provider by type and identifier, retains that provider for the lifetime of the context, and invokes its context creation callback. Unregistration prevents new lookups and contexts while allowing existing contexts to finish. Registry destruction returns a busy status while any registered or retained provider remains.

The registry is an explicit opaque object. No mutable provider registry is exposed as global public state. Registry access is synchronized internally, while provider implementations remain responsible for their own callback state.
