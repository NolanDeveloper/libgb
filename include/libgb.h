/* LIBrary of Growing Buffer.
 *
 * The library provides abstraction over expandable buffer. The user may ask to remove or add bytes
 * to the buffer. The buffer grows exponentially so that asking to grow at linear pace doesn't
 * take O(n^2) time in total because of repeated copying of previous content.
 *
 * One of the common mistakes when working with dynamic block of memory is that after growing
 * the block of memory might be moved to another place. So in conventional tools
 * like manual realloc or std::vector in c++ you must always keep in mind that pointers to elements
 * get spoiled after realloc() or push_back(). The library API is built with this issue in mind.
 * The functions intentionally don't provide easy way of getting pointer to the content of the buffer
 * until the work with it is finished.
 *
 * Workflow:
 * 0. Create library context with libgb_start().
 * 1. Create a growing buffer with libgb_create().
 * 2. Modify or read from buffer.
 * 3. Free resources taken by the buffer. There are 2 options for doing this.
 *    a. libgb_destroy()      -- frees all resources taken by the buffer.
 *    b. libgb_destroy_into() -- frees all resources taken by the buffer except for the buffer itself,
 *                               instead it returns pointer and size of the buffer to the caller.
 * 4. End working with library by calling libgb_finish()
 */

#ifndef LIBJ_LIBGB_H
#define LIBJ_LIBGB_H

#include <stddef.h>

typedef enum {
    LIBGB_ERROR_OK,
    LIBGB_ERROR_BAD_ARGUMENT,
    LIBGB_ERROR_OUT_OF_MEMORY,
    LIBGB_ERROR_INDEX,
} LibgbError;

// Library context. This structure must be passed into every function of the library as the first argument.
typedef struct Libgb_ Libgb;

// The type of growing buffer.
typedef struct LibgbBuffer_ LibgbBuffer;

// Start working with the library, allocate library context and initialize it.
LibgbError libgb_start(Libgb **libgb);

// Finish working with the library, free library context resources.
LibgbError libgb_finish(Libgb **libgb);

// Create growing buffer.
LibgbError libgb_create(Libgb *libgb, LibgbBuffer **buffer);

// Free growing buffer.
LibgbError libgb_destroy(Libgb *libgb, LibgbBuffer **buffer);

// Free resources taken by growing buffer except for the buffer itself.
// Beforehand the memory blocks get reallocated to match exactly with
// its size.
// out      -- output parameter, pointer to the buffer will be stored here.
// out_size -- output parameter, size of the buffer will be stored here.
LibgbError libgb_destroy_into(Libgb *libgb, LibgbBuffer **buffer, char **out, size_t *out_size);

// Get size of the buffer.
//
// Don't confuse with capacity. The buffer may preallocate greater block of memory.
// The size of that block is called capacity while the size of initialized part of that
// block is called size.
LibgbError libgb_get_size(Libgb *libgb, LibgbBuffer *buffer, size_t *size);

// Append null terminated string to the end of the buffer.
LibgbError libgb_append_string(Libgb *libgb, LibgbBuffer *buffer, const char *string);

// Append memory block to the end of the buffer.
LibgbError libgb_append_buffer(Libgb *libgb, LibgbBuffer *buffer, const char *bytes, size_t size);

// Remove a block from the end of the buffer.
LibgbError libgb_drop(Libgb *libgb, LibgbBuffer *buffer, size_t block_size);

// Insert a block into the middle of the buffer.
LibgbError libgb_insert_initialized(
        Libgb *libgb, LibgbBuffer *buffer, size_t offset, size_t block_size, void *userdata,
        void (*initializer)(char *block, size_t block_size, void *userdata));

// Remove a block from the middle of the buffer.
LibgbError libgb_remove(Libgb *libgb, LibgbBuffer *buffer, size_t offset, size_t size);

// Read a block from the middle of the buffer.
LibgbError libgb_read(Libgb *libgb, LibgbBuffer *buffer, size_t offset, char *bytes, size_t size);

// Write a block into the middle of the buffer.
LibgbError libgb_write(Libgb *libgb, LibgbBuffer *buffer, size_t offset, const char *bytes, size_t size);

#endif
