#define StaticArraySize(array) (sizeof(array)/sizeof(array[0]))
#define Radians(degrees) (PI * degrees / 180.0)

#define Kilobytes(kb) ((size_t)kb * 1024)
#define Megabytes(mb) ((size_t)mb * 1024 * 1024)
#define Gigabytes(gb) ((size_t)gb * 1024 * 1024 * 1024)

// Arena Alloactor

extern "C" {
    void *VirtualAlloc(void *, size_t, uint32_t, uint32_t);
}
#define MEM_COMMIT     0x00001000
#define MEM_RESERVE    0x00002000
#define PAGE_READWRITE 0x04

struct Arena {
    uint8_t *buffer;
    size_t   used;
    size_t   capacity;
};

Arena make_arena(size_t size) {
    Arena arena;

    arena.used     = 0;
    arena.capacity = size;
    arena.buffer   = (uint8_t *)VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    assert(arena.buffer);

    return arena;
}

void arena_reset(Arena *arena) {
    memset(arena->buffer, 0, arena->capacity);
    arena->used = 0;
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
    free(array->data);
    memset(array, 0, sizeof(Array<T>));
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

Color lerp_color(Color a, Color b, double t) {
    Color result = a;

    result.r += (b.r - a.r) * t;
    result.g += (b.g - a.g) * t;
    result.b += (b.b - a.b) * t;
    result.a += (b.a - a.a) * t;

    return result;
}

bool rand_bool(double prob_true) {
    double random = rand() / (double)RAND_MAX;
    return random <= prob_true;
}

float rand_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

float rand_range(float start, float end) {
    float result = start;
    result += rand_float() * (end - start);

    return result;
}

// System Time

#if defined(_WIN32)
extern "C" {
    typedef union _LARGE_INTEGER {
        struct {
            uint32_t LowPart;
            uint32_t HighPart;
        } DUMMYSTRUCTNAME;
        struct {
            uint32_t LowPart;
            uint32_t HighPart;
        } u;
        uint64_t QuadPart;
    } LARGE_INTEGER;

    int QueryPerformanceCounter  (LARGE_INTEGER *);
    int QueryPerformanceFrequency(LARGE_INTEGER *);
}

double global_system_timer_frequency = 0;

inline void set_global_system_frequency() {
    LARGE_INTEGER a;
    QueryPerformanceFrequency(&a);
    global_system_timer_frequency = (double)a.QuadPart;
}

inline uint64_t get_system_time() {
    LARGE_INTEGER a;
    QueryPerformanceCounter(&a);
    return a.QuadPart;
}
#endif // defined(_WIN32)
