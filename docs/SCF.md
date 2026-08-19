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

## Безопасная память

Публичный API `scf_secure_allocate` возвращает непрозрачный дескриптор выделения. Дескриптор владеет своим хранилищем и должен освобождаться через `scf_secure_destroy`. Данные, возвращённые `scf_secure_data`, являются заимствованными, действуют только пока дескриптор существует и не должны использоваться после его уничтожения.

Безопасные блоки памяти обнуляются при выделении, могут многократно очищаться явно и повторно очищаются при уничтожении через архитектурную Assembly-примитиву. Выделения нулевого размера допустимы и не содержат данных. Запрошенное выравнивание должно быть степенью двойки и не быть меньше выравнивания указателя. Некорректное выравнивание и переполнение расчёта размера возвращают ошибку без выделения памяти.

Ключи хранят материал в безопасных выделениях. Записи контекстов провайдеров используют безопасные выделения для состояния, принадлежащего фреймворку, а callback-функции провайдеров самостоятельно отвечают за очистку и освобождение принадлежащего им состояния.
