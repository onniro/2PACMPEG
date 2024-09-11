
#if !defined(THANGZ_DOT_H)

#if defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic ignored "-Wlong-long"
    #pragma GCC diagnostic ignored "-Wpointer-arith"
    #pragma GCC diagnostic ignored "-Wwrite-strings"
#elif defined(__clang__)
    #pragma clang diagnostic ignored "-Wlong-long"
    #pragma clang diagnostic ignored "-Wpointer-arith"
    #pragma clang diagnostic ignored "-Wwritable-strings"
#endif

// macros and shit

#define INTERNAL static
#define GLOBAL static
#define LOCAL_STATIC static

#define KILOBYTES(value) (value*1024)
#define MEGABYTES(value) (KILOBYTES(value)*1024)
#define GIGABYTES(Value) (MEGABYTES(Value)*1024)

#if defined(THANGZ_DEBUG)
    #define ASSERT_CRASH(expression) if(!(expression)) *(int *)0 = 1
#else
    #define ASSERT_CRASH(expression)
#endif

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))
#define XORSWAP(x, y) x ^= y; y ^= x; x ^= y

// typedefs

typedef char         s8;
typedef short        s16;
typedef int          s32;
typedef long long    s64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef float f32;
typedef double f64;

typedef int bool32;

typedef struct {
    f32 x;
    f32 y;
} vec2;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} vec3;

#if defined(THANGZ_STRING)

inline u32 string_length(s8 *string) {
    u32 count = 0;
    if(string) {
        while(*string++) {
            ++count;
        }
    }

    return count;
}

inline s8 *string_copy(s8 *destination, s8 *source) {
    while(*destination++ = *source++);

    return destination;
}

inline s8 *string_n_copy(s8 *destination, s8 *source, u32 char_amount) {
    for(u32 index = 0; index < char_amount; ++index) {
        *destination++ = *source++;
    }

    return destination;
}

inline void *mem_copy(void *destination, void *source, u64 bytes) {
    while(bytes) {
        *destination = *(u8 *)source += 1;
        --bytes;
    }

    return destination;
}

inline void *mem_set_value(void *destination, u8 value, u64 bytes) {
    u8 *temp = (u8 *)destination;
    while(bytes) {
        *temp++ = value;
        --bytes;
    }                   

    return destination;
}
#endif

#define THANGZ_DOT_H
#endif
