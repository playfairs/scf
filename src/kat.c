#include <stdatomic.h>
#include <string.h>

#include "internal.h"

typedef struct scf_kat_entry scf_kat_entry;

struct scf_kat_entry
{
    scf_kat_entry *next;
    scf_kat_registry *registry;
    scf_kat_descriptor descriptor;
    char name[SCF_KAT_NAME_MAX];
    scf_secure_allocation *vectors;
    scf_size references;
    uint32_t removed;
};

struct scf_kat_registry
{
    atomic_flag lock;
    scf_provider_registry *providers;
    scf_kat_entry *entries;
};

static void scf_kat_lock(scf_kat_registry *registry)
{
    while (atomic_flag_test_and_set_explicit(
        &registry->lock,
        memory_order_acquire))
    {
    }
}

static void scf_kat_unlock(scf_kat_registry *registry)
{
    atomic_flag_clear_explicit(&registry->lock,
                               memory_order_release);
}

static int
scf_kat_provider_type_valid(scf_provider_type type)
{
    return type >= SCF_PROVIDER_HASH
           && type <= SCF_PROVIDER_FORMAT;
}

static int scf_kat_buffer_valid(scf_const_buffer buffer)
{
    return buffer.size == 0 || buffer.data != NULL;
}

static int scf_kat_add_overflow(scf_size left,
                                scf_size right,
                                scf_size *result)
{
    if (right > SIZE_MAX - left)
    {
        return 1;
    }
    *result = left + right;
    return 0;
}

static scf_status scf_kat_descriptor_validate(
    const scf_kat_descriptor *descriptor)
{
    scf_size name_length;
    const scf_const_buffer buffers[] = {
        descriptor == NULL ? (scf_const_buffer){NULL, 0}
                           : descriptor->vector.input,
        descriptor == NULL ? (scf_const_buffer){NULL, 0}
                           : descriptor->vector.expected,
        descriptor == NULL ? (scf_const_buffer){NULL, 0}
                           : descriptor->vector.key,
        descriptor == NULL ? (scf_const_buffer){NULL, 0}
                           : descriptor->vector.nonce,
        descriptor == NULL ? (scf_const_buffer){NULL, 0}
                           : descriptor->vector.salt,
        descriptor == NULL ? (scf_const_buffer){NULL, 0}
                           : descriptor->vector.auxiliary};

    if (descriptor == NULL
        || !scf_kat_provider_type_valid(
            descriptor->provider_type)
        || descriptor->provider_identifier == 0
        || descriptor->test_identifier == 0
        || descriptor->name == NULL
        || descriptor->name[0] == '\0'
        || descriptor->execute == NULL)
    {
        return SCF_STATUS_TEST_INVALID;
    }
    name_length = strlen(descriptor->name);
    if (name_length >= SCF_KAT_NAME_MAX)
    {
        return SCF_STATUS_NAME_TOO_LONG;
    }
    for (scf_size index = 0;
         index < sizeof(buffers) / sizeof(buffers[0]);
         ++index)
    {
        if (!scf_kat_buffer_valid(buffers[index]))
        {
            return SCF_STATUS_TEST_INVALID;
        }
    }
    return SCF_STATUS_SUCCESS;
}

static scf_kat_entry *
scf_kat_find_locked(scf_kat_registry *registry,
                    scf_provider_type provider_type,
                    uint32_t provider_identifier,
                    uint32_t test_identifier)
{
    for (scf_kat_entry *entry = registry->entries;
         entry != NULL;
         entry = entry->next)
    {
        if (!entry->removed
            && entry->descriptor.provider_type
                   == provider_type
            && entry->descriptor.provider_identifier
                   == provider_identifier
            && entry->descriptor.test_identifier
                   == test_identifier)
        {
            return entry;
        }
    }
    return NULL;
}

static void
scf_kat_remove_locked(scf_kat_registry *registry,
                      scf_kat_entry *target)
{
    scf_kat_entry **entry = &registry->entries;

    while (*entry != NULL && *entry != target)
    {
        entry = &(*entry)->next;
    }
    if (*entry == target)
    {
        target->removed = 1;
        scf_secure_destroy(target->vectors);
        target->vectors = NULL;
    }
}

static void
scf_kat_result_set(const scf_kat_entry *entry,
                   const scf_provider_info *provider,
                   scf_status status,
                   scf_kat_result *result)
{
    result->status = status;
    result->provider_type = entry->descriptor.provider_type;
    result->provider_identifier =
        entry->descriptor.provider_identifier;
    result->test_identifier =
        entry->descriptor.test_identifier;
    memcpy(result->provider_name,
           provider->name,
           sizeof(result->provider_name));
    memcpy(result->test_name,
           entry->name,
           sizeof(result->test_name));
}

static scf_status
scf_kat_status_normalize(scf_status status)
{
    if (status == SCF_STATUS_SUCCESS
        || status == SCF_STATUS_TEST_FAILED
        || status == SCF_STATUS_TEST_INVALID
        || status == SCF_STATUS_TEST_UNSUPPORTED
        || status == SCF_STATUS_INTERNAL_FAILURE)
    {
        return status;
    }
    if (status == SCF_STATUS_UNSUPPORTED)
    {
        return SCF_STATUS_TEST_UNSUPPORTED;
    }
    return SCF_STATUS_INTERNAL_FAILURE;
}

static scf_status scf_kat_copy_vectors(scf_kat_entry *entry)
{
    const scf_const_buffer buffers[] = {
        entry->descriptor.vector.input,
        entry->descriptor.vector.expected,
        entry->descriptor.vector.key,
        entry->descriptor.vector.nonce,
        entry->descriptor.vector.salt,
        entry->descriptor.vector.auxiliary};
    scf_size total = 0;
    scf_size offset = 0;
    scf_buffer storage;

    for (scf_size index = 0;
         index < sizeof(buffers) / sizeof(buffers[0]);
         ++index)
    {
        if (scf_kat_add_overflow(total,
                                 buffers[index].size,
                                 &total))
        {
            return SCF_STATUS_OVERFLOW;
        }
    }
    scf_status status =
        scf_secure_allocate(total, 0, &entry->vectors);
    if (status != SCF_STATUS_SUCCESS)
    {
        return status;
    }
    if (scf_secure_data(entry->vectors, &storage)
        != SCF_STATUS_SUCCESS)
    {
        scf_secure_destroy(entry->vectors);
        entry->vectors = NULL;
        return SCF_STATUS_INTERNAL_FAILURE;
    }
    for (scf_size index = 0;
         index < sizeof(buffers) / sizeof(buffers[0]);
         ++index)
    {
        if (buffers[index].size != 0)
        {
            scf_internal_copy(storage.data + offset,
                              buffers[index].data,
                              buffers[index].size);
            switch (index)
            {
            case 0:
                entry->descriptor.vector.input =
                    (scf_const_buffer){storage.data
                                           + offset,
                                       buffers[index].size};
                break;
            case 1:
                entry->descriptor.vector.expected =
                    (scf_const_buffer){storage.data
                                           + offset,
                                       buffers[index].size};
                break;
            case 2:
                entry->descriptor.vector.key =
                    (scf_const_buffer){storage.data
                                           + offset,
                                       buffers[index].size};
                break;
            case 3:
                entry->descriptor.vector.nonce =
                    (scf_const_buffer){storage.data
                                           + offset,
                                       buffers[index].size};
                break;
            case 4:
                entry->descriptor.vector.salt =
                    (scf_const_buffer){storage.data
                                           + offset,
                                       buffers[index].size};
                break;
            default:
                entry->descriptor.vector.auxiliary =
                    (scf_const_buffer){storage.data
                                           + offset,
                                       buffers[index].size};
                break;
            }
        }
        offset += buffers[index].size;
    }
    return SCF_STATUS_SUCCESS;
}

static scf_status
scf_kat_run_entry_locked(scf_kat_entry *entry,
                         scf_provider_info *provider,
                         scf_kat_result *result)
{
    scf_status status;

    entry->references++;
    scf_kat_unlock(entry->registry);
    status =
        scf_kat_status_normalize(entry->descriptor.execute(
            &entry->descriptor.vector));
    scf_kat_lock(entry->registry);
    entry->references--;
    scf_kat_result_set(entry, provider, status, result);
    if (entry->removed && entry->references == 0)
    {
        scf_kat_remove_locked(entry->registry, entry);
    }
    return status;
}

scf_status
scf_kat_registry_create(scf_provider_registry *providers,
                        scf_kat_registry **registry)
{
    if (providers == NULL || registry == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *registry = scf_internal_alloc(sizeof(**registry));
    if (*registry == NULL)
    {
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    (*registry)->lock = (atomic_flag)ATOMIC_FLAG_INIT;
    (*registry)->providers = providers;
    (*registry)->entries = NULL;
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_kat_register(scf_kat_registry *registry,
                 const scf_kat_descriptor *descriptor)
{
    scf_provider_info provider;
    scf_kat_entry *entry;
    scf_status status;

    status = scf_kat_descriptor_validate(descriptor);
    if (registry == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    if (status != SCF_STATUS_SUCCESS)
    {
        return status;
    }
    status =
        scf_provider_lookup(registry->providers,
                            descriptor->provider_type,
                            descriptor->provider_identifier,
                            &provider);
    if (status != SCF_STATUS_SUCCESS)
    {
        return status == SCF_STATUS_NOT_FOUND
                   ? SCF_STATUS_NOT_FOUND
                   : SCF_STATUS_INTERNAL_FAILURE;
    }
    scf_kat_lock(registry);
    if (scf_kat_find_locked(registry,
                            descriptor->provider_type,
                            descriptor->provider_identifier,
                            descriptor->test_identifier)
        != NULL)
    {
        scf_kat_unlock(registry);
        return SCF_STATUS_DUPLICATE;
    }
    entry = scf_internal_alloc(sizeof(*entry));
    if (entry == NULL)
    {
        scf_kat_unlock(registry);
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    memset(entry, 0, sizeof(*entry));
    entry->registry = registry;
    entry->descriptor = *descriptor;
    memcpy(entry->name,
           descriptor->name,
           strlen(descriptor->name) + 1);
    entry->descriptor.name = entry->name;
    status = scf_kat_copy_vectors(entry);
    if (status != SCF_STATUS_SUCCESS)
    {
        scf_internal_clear(entry, sizeof(*entry));
        scf_internal_free(entry);
        scf_kat_unlock(registry);
        return status;
    }
    entry->next = registry->entries;
    registry->entries = entry;
    scf_kat_unlock(registry);
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_kat_unregister(scf_kat_registry *registry,
                   scf_provider_type provider_type,
                   uint32_t provider_identifier,
                   uint32_t test_identifier)
{
    scf_kat_entry *entry;

    if (registry == NULL
        || !scf_kat_provider_type_valid(provider_type)
        || provider_identifier == 0 || test_identifier == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_kat_lock(registry);
    entry = scf_kat_find_locked(registry,
                                provider_type,
                                provider_identifier,
                                test_identifier);
    if (entry == NULL)
    {
        scf_kat_unlock(registry);
        return SCF_STATUS_NOT_FOUND;
    }
    entry->removed = 1;
    if (entry->references == 0)
    {
        scf_kat_remove_locked(registry, entry);
    }
    scf_kat_unlock(registry);
    return SCF_STATUS_SUCCESS;
}

scf_status scf_kat_run(scf_kat_registry *registry,
                       scf_provider_type provider_type,
                       uint32_t provider_identifier,
                       uint32_t test_identifier,
                       scf_kat_result *result)
{
    scf_provider_info provider;
    scf_kat_entry *entry;
    scf_status status;

    if (registry == NULL || result == NULL
        || !scf_kat_provider_type_valid(provider_type)
        || provider_identifier == 0 || test_identifier == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    memset(result, 0, sizeof(*result));
    result->status = SCF_STATUS_NOT_FOUND;
    result->provider_type = provider_type;
    result->provider_identifier = provider_identifier;
    result->test_identifier = test_identifier;
    status = scf_provider_lookup(registry->providers,
                                 provider_type,
                                 provider_identifier,
                                 &provider);
    if (status != SCF_STATUS_SUCCESS)
    {
        result->status = status == SCF_STATUS_NOT_FOUND
                             ? SCF_STATUS_NOT_FOUND
                             : SCF_STATUS_INTERNAL_FAILURE;
        return result->status;
    }
    scf_kat_lock(registry);
    entry = scf_kat_find_locked(registry,
                                provider_type,
                                provider_identifier,
                                test_identifier);
    if (entry == NULL)
    {
        scf_kat_unlock(registry);
        return SCF_STATUS_NOT_FOUND;
    }
    status =
        scf_kat_run_entry_locked(entry, &provider, result);
    scf_kat_unlock(registry);
    return status;
}

scf_status
scf_kat_run_provider(scf_kat_registry *registry,
                     scf_provider_type provider_type,
                     uint32_t provider_identifier,
                     scf_kat_result *result,
                     scf_size *executed)
{
    scf_provider_info provider;
    scf_status overall = SCF_STATUS_SUCCESS;
    scf_size count = 0;

    if (registry == NULL || result == NULL
        || executed == NULL
        || !scf_kat_provider_type_valid(provider_type)
        || provider_identifier == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    memset(result, 0, sizeof(*result));
    *executed = 0;
    if (scf_provider_lookup(registry->providers,
                            provider_type,
                            provider_identifier,
                            &provider)
        != SCF_STATUS_SUCCESS)
    {
        result->status = SCF_STATUS_NOT_FOUND;
        result->provider_type = provider_type;
        result->provider_identifier = provider_identifier;
        return result->status;
    }
    scf_kat_lock(registry);
    scf_kat_entry *entry = registry->entries;
    while (entry != NULL)
    {
        scf_kat_entry *next = entry->next;
        if (!entry->removed
            && entry->descriptor.provider_type
                   == provider_type
            && entry->descriptor.provider_identifier
                   == provider_identifier)
        {
            scf_status status =
                scf_kat_run_entry_locked(entry,
                                         &provider,
                                         result);
            count++;
            if (status != SCF_STATUS_SUCCESS
                && overall == SCF_STATUS_SUCCESS)
            {
                overall = status;
            }
        }
        entry = next;
    }
    scf_kat_unlock(registry);
    *executed = count;
    if (count == 0)
    {
        result->status = SCF_STATUS_TEST_NO_VECTORS;
        return result->status;
    }
    return overall;
}

scf_status scf_kat_run_all(scf_kat_registry *registry,
                           scf_kat_result *result,
                           scf_size *executed)
{
    scf_status overall = SCF_STATUS_SUCCESS;
    scf_size count = 0;

    if (registry == NULL || result == NULL
        || executed == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    memset(result, 0, sizeof(*result));
    *executed = 0;
    scf_kat_entry *entry = registry->entries;
    scf_kat_lock(registry);
    while (entry != NULL)
    {
        scf_kat_entry *next = entry->next;
        scf_provider_info provider;
        scf_status status;

        scf_status lookup = scf_provider_lookup(
            registry->providers,
            entry->descriptor.provider_type,
            entry->descriptor.provider_identifier,
            &provider);
        if (lookup != SCF_STATUS_SUCCESS)
        {
            result->status = SCF_STATUS_NOT_FOUND;
            result->provider_type =
                entry->descriptor.provider_type;
            result->provider_identifier =
                entry->descriptor.provider_identifier;
            result->test_identifier =
                entry->descriptor.test_identifier;
            memcpy(result->test_name,
                   entry->name,
                   sizeof(result->test_name));
            status = SCF_STATUS_NOT_FOUND;
        }
        else
        {
            status = scf_kat_run_entry_locked(entry,
                                              &provider,
                                              result);
        }
        count++;
        if (status != SCF_STATUS_SUCCESS
            && overall == SCF_STATUS_SUCCESS)
        {
            overall = status;
        }
        entry = next;
    }
    scf_kat_unlock(registry);
    *executed = count;
    return count == 0 ? SCF_STATUS_TEST_NO_VECTORS
                      : overall;
}

scf_status
scf_kat_registry_destroy(scf_kat_registry *registry)
{
    if (registry == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_kat_entry *entry;

    scf_kat_lock(registry);
    for (entry = registry->entries; entry != NULL;
         entry = entry->next)
    {
        if (!entry->removed || entry->references != 0)
        {
            scf_kat_unlock(registry);
            return SCF_STATUS_BUSY;
        }
    }
    entry = registry->entries;
    while (entry != NULL)
    {
        scf_kat_entry *next = entry->next;
        scf_secure_destroy(entry->vectors);
        scf_internal_clear(entry, sizeof(*entry));
        scf_internal_free(entry);
        entry = next;
    }
    registry->entries = NULL;
    scf_kat_unlock(registry);
    scf_internal_clear(registry, sizeof(*registry));
    scf_internal_free(registry);
    return SCF_STATUS_SUCCESS;
}
