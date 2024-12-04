#define StaticArraySize(array) (sizeof(array)/sizeof(array[0]))

#define Radians(degrees) (PI * degrees / 180.0)
#define Degrees(radians) (180.0 * radians / PI)

// Technically KiB, MiB, GiB
#define Kilobytes(kb) ((size_t)kb * 1024)
#define Megabytes(mb) ((size_t)mb * 1024 * 1024)
#define Gigabytes(gb) ((size_t)gb * 1024 * 1024 * 1024)

#define BlenderPosition3D(x, y, z) {(x), (z), -(y)}
#define BlenderPosition2D(x, y)    {(x), -(y)}

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

void arena_print_used(Arena arena) {
    printf("Arena: Used %zu/%zu (%f%%)\n",
           arena.used,
           arena.capacity,
           (double)arena.used / (double)arena.capacity);
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

Rectangle enlarge_rectangle(Rectangle a, float amount) {
    Rectangle result = a;

    result.x -= amount;
    result.y -= amount;
    result.width += amount * 2;
    result.height += amount * 2;

    return result;
}

void print_cam(Camera3D *cam) {
    printf("position = {%ff, %ff, %ff}\ntarget = {%ff, %ff, %ff}\n\n",
           cam->position.x,
           cam->position.y,
           cam->position.z,
           cam->target.x,
           cam->target.y,
           cam->target.z);
}

// TODO: Ensure this works with vsync, and target framerates such
//       as 165hz, 144hz...
float get_frame_time_clamped() {
    float dt = GetFrameTime();

    // Round FPS to the nearest 10; 30, 40, 50, 60, 70...
    float fps = 1.f / dt;
    fps /= 10;

    fps = 10 * round(fps);

    // Then set dt to that exact value. (16.6666... ms for instance)
    dt = 1.f / fps;

    return dt;
}

float go_to(float a, float b, float speed) {
    float dir;

    if (a == b) return a;

    if (b < a)
        dir = -1;
    else
        dir = +1;

    float result = a + speed * dir;

    if (b < a)
        result = Clamp(result, b, a);
    else
        result = Clamp(result, a, b);

    return result;
}

inline Vector2 int_vec2(Vector2 in) {
    in.x = (int)in.x;
    in.y = (int)in.y;

    return in;
}

bool compare_colors(Color a, Color b) {
    return (a.r == b.r && a.g == b.g && a.b && b.b);
}

bool vector3_same(Vector3 a, Vector3 b) {
    if (a.x == b.x && a.y == b.y && a.z == b.z)
        return true;
    return false;
}

Color ColorScale(Color a, float b) {
    a.r *= b;
    a.g *= b;
    a.b *= b;

    return a;
}

void print_rectangle(Rectangle rect) {
    printf("{%f, %f, %f, %f}", rect.x, rect.y, rect.width, rect.height);
}

void print_vec2(Vector2 vec) {
    printf("%f, %f", vec.x, vec.y);
}

void print_vec3(Vector3 vec) {
    printf("%f, %f, %f\n", vec.x, vec.y, vec.z);
}

Rectangle integer_rectangle(Rectangle rect) {
    Rectangle result;

    result.x      = (int)rect.x;
    result.y      = (int)rect.y;
    result.width  = (int)rect.width;
    result.height = (int)rect.height;

    return result;
}

void seconds_to_minutes_and_seconds(float time, char *output) {
    int minutes = (int)(time / 60);
    int seconds = time - minutes * 60;

    sprintf(output, "%d:%02d", minutes, seconds);
}

BoundingBox BoundingBoxMove(BoundingBox box, Vector3 offset) {
    BoundingBox result;

    result.min = Vector3Add(box.min, offset);
    result.max = Vector3Add(box.max, offset);

    return result;
}

bool Vector3InBoundingBox(Vector3 point, BoundingBox box) {
    Vector3 min = box.min;
    Vector3 max = box.max;

    return (point.x >= min.x &&
            point.y >= min.y &&
            point.z >= min.z &&
            point.x <= max.x &&
            point.y <= max.y &&
            point.z <= max.z);
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

// An approximation.
// If you're feeling frisky, replace it with lerp(a, b, 1 - f^dt),
// where f = remaining percentage after one second.
float lerp_dt(float a, float b, float t, float dt) {
    return Lerp(a, b, 60 * dt * t);
}

Vector3 LerpVec3(Vector3 a, Vector3 b, double t) {
    a.x = a.x + (b.x - a.x) * t;
    a.y = a.y + (b.y - a.y) * t;
    a.z = a.z + (b.z - a.z) * t;

    return a;
}

// top_speed_frame -> Maximum distance to travel in 1 frame.
// t = 0 -> start (result = a)
// t = 1 -> end   (result = b)
// Caller should keep t (probably as a static variable) that
// persists over frames
Vector3 GotoSmoothVec3(Vector3 a, Vector3 b, float top_speed_frame, double t) {
    double proportion = 1 - (t-1)*(t-1); // 2t - t^2

    double dist = Vector3Distance(b, a);

    double to_travel = proportion * dist;

    // limit the distance to travel to top speed.
    if (to_travel > top_speed_frame) {
        to_travel = top_speed_frame;

        // recalculate proportion based on that.
        proportion = to_travel / dist;
    }

    Vector3 result = LerpVec3(a, b, proportion);

    return result;
}

// t = (0.0, 1.0)
double smoothstep(double start, double end, double t) {
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    double fac = 3 * t * t - 2 * t * t * t;

    return start + (end - start) * fac;
}

Vector3 lerp_vector3(Vector3 a, Vector3 b, float t) {
    Vector3 result;

    result.x = Lerp(a.x, b.x, t);
    result.y = Lerp(a.y, b.y, t);
    result.z = Lerp(a.z, b.z, t);

    return result;
}

Vector3 smoothstep_vector3(Vector3 start, Vector3 end, double t) {
    Vector3 result = start;

    result.x = (float)smoothstep((double)start.x, (double)end.x, t);
    result.y = (float)smoothstep((double)start.y, (double)end.y, t);
    result.z = (float)smoothstep((double)start.z, (double)end.z, t);

    return result;
}

Color lerp_color(Color a, Color b, double t) {
    Color result = a;

    result.r += (b.r - a.r) * t;
    result.g += (b.g - a.g) * t;
    result.b += (b.b - a.b) * t;
    result.a += (b.a - a.a) * t;

    return result;
}

// hue        - angle in degrees
// saturation - 0.0 to 1.0
// value      - 0.0 to 1.0
Color hsv(float hue, float saturation, float value) {
    double      hh, p, q, t, ff;
    long        i;
    Color       out;

    if(saturation <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = value * 255;
        out.g = value * 255;
        out.b = value * 255;
        return out;
    }

    hh = hue;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = value * (1.0 - saturation);
    q = value * (1.0 - (saturation * ff));
    t = value * (1.0 - (saturation * (1.0 - ff)));

    switch(i) {
        case 0:
            out.r = 255 * value;
            out.g = 255 * t;
            out.b = 255 * p;
            break;
        case 1:
            out.r = 255 * q;
            out.g = 255 * value;
            out.b = 255 * p;
            break;
        case 2:
            out.r = 255 * p;
            out.g = 255 * value;
            out.b = 255 * t;
            break;

        case 3:
            out.r = 255 * p;
            out.g = 255 * q;
            out.b = 255 * value;
            break;
        case 4:
            out.r = 255 * t;
            out.g = 255 * p;
            out.b = 255 * value;
            break;
        case 5:
        default:
            out.r = 255 * value;
            out.g = 255 * p;
            out.b = 255 * q;
            break;
    }

    return out;     
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

int rand_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

Color random_color(int seed) {
    Color result;

    srand(seed);

    float hue = rand_range(0, 360);
    float saturation = 0.25;
    float value = 1;

    result = hsv(hue, saturation, value);

    return result;
}

bool colors_equal(Color a, Color b) {
    return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}

bool string_compare(const char *major, const char *minor) {
    int major_length = (int)strlen(major);
    int minor_length = (int)strlen(minor);

    if (major_length < minor_length)
        return false;

    for (int i = 0; i < minor_length; i++) {
        if (major[i] != minor[i])
            return false;
    }

    return true;
}

struct Find_Word_Result {
    int start_index;
    int end_index;
};

// finds the n'th occurrence. starts at 1.
Find_Word_Result find_word(const char *string, const char *word, int n) {
    Find_Word_Result result = {};

    result.start_index = -1;
    result.end_index = -1;

    size_t word_len = strlen(word);

    int current = 0;

    int string_length = (int)strlen(string);
    for (int i = 0; i <= string_length - word_len; i++) {
        if (string_compare(string + i, word)) {
            if (i == 0 || (i > 0 && isspace(string[i-1])) &&
                (isspace(string[i+word_len]) || i+word_len == string_length))
            {
                current++;

                if (n == current) {
                    result.start_index = i;
                    result.end_index = i + (int)strlen(word) - 1;

                    return result;
                }
            }
        }
    }

    assert(false);

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

inline void set_global_system_timer_frequency() {
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
