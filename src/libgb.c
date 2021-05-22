#include <libgb.h>

#include <stdlib.h>
#include <string.h>

struct Libgb_ {
    int dummy;
};

struct LibgbBuffer_ {
    char *ptr;
    size_t size;
    size_t capacity;
};

#define INITIAL_CAPACITY (sizeof(void *))

#define E libgb_handle_internal_error

static LibgbError libgb_handle_internal_error(LibgbError err) {
    switch (err) {
    case LIBGB_ERROR_OK:
        return LIBGB_ERROR_OK;
    case LIBGB_ERROR_BAD_ARGUMENT:
        abort();
    case LIBGB_ERROR_OUT_OF_MEMORY:
        return LIBGB_ERROR_OUT_OF_MEMORY;
    case LIBGB_ERROR_INDEX:
        return LIBGB_ERROR_INDEX;
    }
    abort();
}

static LibgbError reserve(Libgb *libgb, LibgbBuffer *buffer, size_t required_capacity) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (required_capacity <= buffer->capacity) {
        goto end;
    }
    size_t new_capacity = buffer->capacity;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }
    void *new_ptr = realloc(buffer->ptr, new_capacity);
    if (new_capacity && !new_ptr) {
        err = LIBGB_ERROR_OUT_OF_MEMORY;
        goto end;
    }
    buffer->ptr = new_ptr;
end:
    return err;
}

LibgbError libgb_start(Libgb **libgb) {
    LibgbError err = LIBGB_ERROR_OK;
    Libgb *result = NULL;
    if (!libgb) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    result = malloc(sizeof(Libgb));
    if (!result) {
        err = LIBGB_ERROR_OUT_OF_MEMORY;
        goto end;
    }
    *libgb = result;
    result = NULL;
end:
    return err;
}

LibgbError libgb_finish(Libgb **libgb) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (!*libgb) {
        goto end;
    }
    free(*libgb);
end:
    return err;
}

LibgbError libgb_create(Libgb *libgb, LibgbBuffer **buffer) {
    LibgbError err = LIBGB_ERROR_OK;
    void *ptr = NULL;
    LibgbBuffer *result = NULL;
    if (!libgb || !buffer) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    ptr = malloc(INITIAL_CAPACITY);
    if (!ptr) {
        err = LIBGB_ERROR_OUT_OF_MEMORY;
        goto end;
    }
    result = malloc(sizeof(LibgbBuffer));
    if (!result) {
        err = LIBGB_ERROR_OUT_OF_MEMORY;
        goto end;
    }
    result->size = 0;
    result->capacity = INITIAL_CAPACITY;
    result->ptr = ptr;
    *buffer = result;
    result = NULL;
    ptr = NULL;
end:
    free(result);
    free(ptr);
    return err;
}

LibgbError libgb_destroy(Libgb *libgb, LibgbBuffer **buffer) {
    LibgbError err = LIBGB_ERROR_OK;
    char *ptr = NULL;
    size_t size;
    if (!libgb || !buffer) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    err = E(libgb_destroy_into(libgb, buffer, &ptr, &size));
    if (err) goto end;
end:
    free(ptr);
    return err;
}

LibgbError libgb_destroy_into(Libgb *libgb, LibgbBuffer **buffer, char **out, size_t *out_size) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (*buffer) {
        char *compacted = realloc((*buffer)->ptr, (*buffer)->size);
        if (!(*buffer)->size || compacted) {
            (*buffer)->ptr = compacted;
        }
        *out = (*buffer)->ptr;
        *out_size = (*buffer)->size;
        free(*buffer);
        *buffer = NULL;
    }
end:
    return err;
}

LibgbError libgb_get_size(Libgb *libgb, LibgbBuffer *buffer, size_t *size) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer || !size) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    *size = buffer->size;
end:
    return err;
}

LibgbError libgb_append_string(Libgb *libgb, LibgbBuffer *buffer, const char *string) {
    if (!string) {
        return LIBGB_ERROR_BAD_ARGUMENT;
    }
    return libgb_append_buffer(libgb, buffer, string, strlen(string) + 1);
}

static void initialize_buffer(char *block, size_t block_size, void *userdata) {
    memcpy(block, (const char *) userdata, block_size);
}

LibgbError libgb_append_buffer(Libgb *libgb, LibgbBuffer *buffer, const char *bytes, size_t size) {
    return libgb_insert_initialized(libgb, buffer, buffer->size, size, (void *) bytes, initialize_buffer);
}

LibgbError libgb_append_char(Libgb *libgb, LibgbBuffer *buffer, char c) {
    return libgb_append_buffer(libgb, buffer, &c, 1);
}

LibgbError libgb_drop(Libgb *libgb, LibgbBuffer *buffer, size_t block_size) {
    if (!buffer) {
        return LIBGB_ERROR_BAD_ARGUMENT;
    }
    return libgb_remove(libgb, buffer, buffer->size - block_size, block_size);
}

LibgbError libgb_insert_initialized(
        Libgb *libgb, LibgbBuffer *buffer, size_t offset, size_t block_size, void *userdata,
        void (*initializer)(char *block, size_t block_size, void *userdata)) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer || !initializer) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (!block_size) {
        goto end;
    }
    if (buffer->size < offset) {
        err = LIBGB_ERROR_INDEX;
        goto end;
    }
    err = E(reserve(libgb, buffer, offset + block_size));
    if (err) goto end;
    memmove(buffer->ptr + offset + block_size, buffer->ptr + offset, buffer->size - offset);
    initializer(buffer->ptr + offset, block_size, userdata);
    buffer->size += block_size;
end:
    return err;
}

LibgbError libgb_remove(Libgb *libgb, LibgbBuffer *buffer, size_t offset, size_t size) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (!size) {
        goto end;
    }
    if (buffer->size < offset + size) {
        err = LIBGB_ERROR_INDEX;
        goto end;
    }
    memmove(buffer->ptr + offset, buffer->ptr + offset + size, buffer->size - offset - size);
    buffer->size -= size;
end:
    return err;
}

LibgbError libgb_read(Libgb *libgb, LibgbBuffer *buffer, size_t offset, char *bytes, size_t size) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer || (size && !bytes)) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (!size) {
        goto end;
    }
    if (buffer->size < offset + size) {
        err = LIBGB_ERROR_INDEX;
        goto end;
    }
    memcpy(bytes, buffer->ptr + offset, size);
end:
    return err;
}

LibgbError libgb_write(Libgb *libgb, LibgbBuffer *buffer, size_t offset, const char *bytes, size_t size) {
    LibgbError err = LIBGB_ERROR_OK;
    if (!libgb || !buffer || (size && !bytes)) {
        err = LIBGB_ERROR_BAD_ARGUMENT;
        goto end;
    }
    if (!size) {
        goto end;
    }
    if (buffer->size < offset + size) {
        err = LIBGB_ERROR_INDEX;
        goto end;
    }
    memcpy(buffer->ptr + offset, bytes, size);
end:
    return err;
}
