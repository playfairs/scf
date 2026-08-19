#include <stdatomic.h>
#include <string.h>

#include "internal.h"

typedef struct scf_provider_entry scf_provider_entry;

struct scf_provider_entry
{
    scf_provider_entry *next;
    scf_provider_registry *registry;
    scf_provider_descriptor descriptor;
    char name[SCF_PROVIDER_NAME_MAX];
    scf_size references;
    uint32_t removed;
};

struct scf_provider_registry
{
    atomic_flag lock;
    scf_provider_entry *entries;
};

struct scf_provider_context
{
    scf_provider_entry *entry;
    void *state;
    scf_secure_allocation *memory;
};

static int scf_provider_type_valid(scf_provider_type type)
{
    return type >= SCF_PROVIDER_HASH
           && type <= SCF_PROVIDER_FORMAT;
}

static void
scf_provider_lock(scf_provider_registry *registry)
{
    while (atomic_flag_test_and_set_explicit(
        &registry->lock,
        memory_order_acquire))
    {
    }
}

static void
scf_provider_unlock(scf_provider_registry *registry)
{
    atomic_flag_clear_explicit(&registry->lock,
                               memory_order_release);
}

static scf_status scf_provider_descriptor_validate(
    const scf_provider_descriptor *descriptor)
{
    scf_size length;

    if (descriptor == NULL
        || !scf_provider_type_valid(descriptor->type)
        || descriptor->identifier == 0
        || descriptor->name == NULL
        || descriptor->name[0] == '\0'
        || descriptor->context_create == NULL
        || descriptor->context_destroy == NULL
        || (descriptor->capabilities
            & SCF_PROVIDER_CAP_CONTEXT)
               == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    length = strlen(descriptor->name);
    if (length >= SCF_PROVIDER_NAME_MAX)
    {
        return SCF_STATUS_NAME_TOO_LONG;
    }
    return SCF_STATUS_SUCCESS;
}

static scf_provider_entry *
scf_provider_find_locked(scf_provider_registry *registry,
                         scf_provider_type type,
                         uint32_t identifier)
{
    for (scf_provider_entry *entry = registry->entries;
         entry != NULL;
         entry = entry->next)
    {
        if (!entry->removed
            && entry->descriptor.type == type
            && entry->descriptor.identifier == identifier)
        {
            return entry;
        }
    }
    return NULL;
}

static void
scf_provider_info_copy(const scf_provider_entry *entry,
                       scf_provider_info *info)
{
    info->type = entry->descriptor.type;
    info->identifier = entry->descriptor.identifier;
    info->capabilities = entry->descriptor.capabilities;
    memcpy(info->name, entry->name, sizeof(info->name));
}

static void scf_provider_remove_entry_locked(
    scf_provider_registry *registry,
    scf_provider_entry *target)
{
    scf_provider_entry **entry = &registry->entries;

    while (*entry != NULL && *entry != target)
    {
        entry = &(*entry)->next;
    }
    if (*entry == target)
    {
        *entry = target->next;
        scf_internal_clear(target, sizeof(*target));
        scf_internal_free(target);
    }
}

scf_status scf_provider_registry_create(
    scf_provider_registry **registry)
{
    if (registry == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *registry = scf_internal_alloc(sizeof(**registry));
    if (*registry == NULL)
    {
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    (*registry)->lock = (atomic_flag)ATOMIC_FLAG_INIT;
    (*registry)->entries = NULL;
    return SCF_STATUS_SUCCESS;
}

scf_status scf_provider_register(
    scf_provider_registry *registry,
    const scf_provider_descriptor *descriptor)
{
    scf_provider_entry *entry;
    scf_status status;

    status = scf_provider_descriptor_validate(descriptor);
    if (status != SCF_STATUS_SUCCESS || registry == NULL)
    {
        return registry == NULL
                   ? SCF_STATUS_INVALID_ARGUMENT
                   : status;
    }
    scf_provider_lock(registry);
    if (scf_provider_find_locked(registry,
                                 descriptor->type,
                                 descriptor->identifier)
        != NULL)
    {
        scf_provider_unlock(registry);
        return SCF_STATUS_DUPLICATE;
    }
    entry = scf_internal_alloc(sizeof(*entry));
    if (entry == NULL)
    {
        scf_provider_unlock(registry);
        return SCF_STATUS_ALLOCATION_FAILED;
    }
    memset(entry, 0, sizeof(*entry));
    entry->registry = registry;
    entry->descriptor = *descriptor;
    memcpy(entry->name,
           descriptor->name,
           strlen(descriptor->name) + 1);
    entry->descriptor.name = entry->name;
    entry->next = registry->entries;
    registry->entries = entry;
    scf_provider_unlock(registry);
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_provider_unregister(scf_provider_registry *registry,
                        scf_provider_type type,
                        uint32_t identifier)
{
    scf_provider_entry *entry;

    if (registry == NULL || !scf_provider_type_valid(type)
        || identifier == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_provider_lock(registry);
    entry = scf_provider_find_locked(registry,
                                     type,
                                     identifier);
    if (entry == NULL)
    {
        scf_provider_unlock(registry);
        return SCF_STATUS_NOT_FOUND;
    }
    entry->removed = 1;
    if (entry->references == 0)
    {
        scf_provider_remove_entry_locked(registry, entry);
    }
    scf_provider_unlock(registry);
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_provider_lookup(const scf_provider_registry *registry,
                    scf_provider_type type,
                    uint32_t identifier,
                    scf_provider_info *info)
{
    scf_provider_entry *entry;

    if (registry == NULL || info == NULL
        || !scf_provider_type_valid(type)
        || identifier == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_provider_lock((scf_provider_registry *)registry);
    entry = scf_provider_find_locked(
        (scf_provider_registry *)registry,
        type,
        identifier);
    if (entry == NULL)
    {
        scf_provider_unlock(
            (scf_provider_registry *)registry);
        return SCF_STATUS_NOT_FOUND;
    }
    scf_provider_info_copy(entry, info);
    scf_provider_unlock((scf_provider_registry *)registry);
    return SCF_STATUS_SUCCESS;
}

scf_status
scf_provider_context_create(scf_provider_registry *registry,
                            scf_provider_type type,
                            uint32_t identifier,
                            scf_const_buffer parameters,
                            scf_provider_context **context)
{
    scf_provider_entry *entry;
    scf_status status;
    scf_secure_allocation *memory = NULL;
    scf_status memory_status;

    if (registry == NULL || context == NULL
        || !scf_internal_buffer_valid(parameters)
        || !scf_provider_type_valid(type)
        || identifier == 0)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    *context = NULL;
    scf_provider_lock(registry);
    entry = scf_provider_find_locked(registry,
                                     type,
                                     identifier);
    if (entry == NULL)
    {
        scf_provider_unlock(registry);
        return SCF_STATUS_NOT_FOUND;
    }
    entry->references++;
    scf_provider_unlock(registry);

    memory_status =
        scf_secure_allocate(sizeof(**context),
                            _Alignof(max_align_t),
                            &memory);
    if (memory_status != SCF_STATUS_SUCCESS)
    {
        scf_provider_lock(registry);
        entry->references--;
        scf_provider_unlock(registry);
        return memory_status;
    }
    scf_buffer context_memory;
    scf_secure_data(memory, &context_memory);
    *context =
        (scf_provider_context *)(void *)context_memory.data;
    (*context)->memory = memory;
    (*context)->entry = entry;
    (*context)->state = NULL;
    status = entry->descriptor.context_create(
        parameters,
        &(*context)->state);
    if (status != SCF_STATUS_SUCCESS)
    {
        scf_provider_context_destroy(*context);
        *context = NULL;
    }
    return status;
}

scf_status scf_provider_context_info(
    const scf_provider_context *context,
    scf_provider_info *info)
{
    if (context == NULL || info == NULL
        || context->entry == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_provider_info_copy(context->entry, info);
    return SCF_STATUS_SUCCESS;
}

void scf_provider_context_destroy(
    scf_provider_context *context)
{
    scf_provider_entry *entry;
    scf_provider_registry *registry;

    if (context == NULL)
    {
        return;
    }
    entry = context->entry;
    registry = entry->registry;
    entry->descriptor.context_destroy(context->state);
    scf_secure_allocation *memory = context->memory;
    scf_secure_destroy(memory);
    scf_provider_lock(registry);
    entry->references--;
    if (entry->removed && entry->references == 0)
    {
        scf_provider_remove_entry_locked(registry, entry);
    }
    scf_provider_unlock(registry);
}

scf_status scf_provider_registry_destroy(
    scf_provider_registry *registry)
{
    if (registry == NULL)
    {
        return SCF_STATUS_INVALID_ARGUMENT;
    }
    scf_provider_lock(registry);
    if (registry->entries != NULL)
    {
        scf_provider_unlock(registry);
        return SCF_STATUS_BUSY;
    }
    scf_provider_unlock(registry);
    scf_internal_clear(registry, sizeof(*registry));
    scf_internal_free(registry);
    return SCF_STATUS_SUCCESS;
}