#define StaticArraySize(array) (sizeof(array)/sizeof(array[0]))
#define Radians(degrees) (PI * degrees / 180.0)

// Arena Alloactor

struct Arena {
    uint8_t *buffer;
    size_t   used;
    size_t   capacity;
};

Arena make_arena(size_t initial_size) {
    Arena arena;

    arena.used     = 0;
    arena.capacity = initial_size;
    arena.buffer   = (uint8_t *)calloc(initial_size, sizeof(arena.buffer[0]));

    return arena;
}

void arena_free(Arena *arena) {
    if (arena->buffer)
        free(arena->buffer);
    memset(arena, 0, sizeof(Arena));
}

void *arena_push(Arena *arena, size_t size) {
    void *result = 0;

    if (arena->used + size >= arena->capacity) {
        assert(false);
    } else {
        result = (arena->buffer + arena->used);
        arena->used += size;
    }

    return result;
}

void arena_pop(Arena *arena, size_t size) {
    if (arena->used >= size) {
        arena->used -= size;
    } else {
        assert(false);
    }
}

// Dynamic Array

template <typename T>
struct Array {
    T *data;
    size_t length;
    size_t capacity;
};

template <typename T>
Array<T> make_array(size_t initial_capacity) {
    Array<T> result = {};

    result.capacity = initial_capacity;
    result.length   = 0;
    result.data     = (T *)calloc(result.capacity, sizeof(T));

    return result;
}

template <typename T>
void array_free(Array<T> *array) {
    assert(array->data);
    if (array->data == nullptr) return;

    memset(array->data, 0, sizeof(T)*array->length);

    free(array->data);

    array->data     = 0;
    array->length   = 0;
    array->capacity = 0;
}

template <typename T>
void array_add(Array<T> *array, T value) {
    assert(array->data);
    if (array->data == nullptr) return;

    if (array->length + 1 > array->capacity) {
        // Reallocate
        array->capacity *= 2;

        T *new_data = (T *) calloc(array->capacity, sizeof(T));
        memcpy(new_data, array->data, array->length * sizeof(T));
        free(array->data);
        array->data = new_data;
    }

    array->data[array->length++] = value;
}

// Swap the last member with index.
template <typename T>
void array_remove(Array<T> *array, int index) {
    assert(array->data);
    if (array->data == nullptr) return;

    assert(index >= 0);
    assert(index < array->length);

    array->data[index] = array->data[--array->length];
}

template <typename T>
void array_print(Array<T> array, const char *t_format_specifier) {
    if (array->data == nullptr) {
        printf("Array is uninitialized.\n");
        return;
    }

    printf("Length = %zu, Capacity = %zu\n",
           array.length,
           array.capacity);

    for (int i = 0; i < array.length; i++) {
        printf(t_format_specifier, array.data[i]);
        if (i != array.length-1)
            printf(",\n");
    }
}

// Misc

void print_rectangle(Rectangle rect) {
    printf("{%f, %f, %f, %f}", rect.x, rect.y, rect.width, rect.height);
}

Texture2D load_texture(const char *filename) {
    Texture2D result;

    assert(filename);
    result = LoadTexture(filename);
    assert(IsTextureReady(result));

    return result;
}

int sign(float a) {
    if (a < 0) return -1;
    if (a > 0) return +1;
    return 0;
}
