// compat_c.h - Central compatibility layer for migrating from C++ to C

#ifndef COMPAT_C_H
#define COMPAT_C_H

#include <stddef.h>   // for size_t
#include <stdlib.h>   // for NULL (though we redefine it safely)

// ================================================================
// SAFE replacements for std::min, std::max, std::clamp
// Using static inline functions avoids macro double-evaluation bugs.
// ================================================================

static inline int min_int(int a, int b) {
    return (a < b) ? a : b;
}

static inline int max_int(int a, int b) {
    return (a > b) ? a : b;
}

static inline int clamp_int(int val, int lo, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

static inline float min_float(float a, float b) {
    return (a < b) ? a : b;
}

static inline float max_float(float a, float b) {
    return (a > b) ? a : b;
}

static inline float clamp_float(float val, float lo, float hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

static inline double min_double(double a, double b) {
    return (a < b) ? a : b;
}

static inline double max_double(double a, double b) {
    return (a > b) ? a : b;
}

static inline double clamp_double(double val, double lo, double hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

// Generic macro versions
// WARNING: These evaluate arguments twice, so avoid using with side-effects.
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define CLAMP(v,l,h) ((v) < (l) ? (l) : ((v) > (h) ? (h) : (v)))

// ================================================================
// 2. General utility macros
// ================================================================

// Number of elements in a static array (e.g., ARRAY_SIZE(myArray))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Silence "unused variable/parameter" warnings (useful during transition)
#define UNUSED(x) ((void)(x))

// ================================================================
// 3. C++/C linkage helpers
//    EXTERN_C_BEGIN / EXTERN_C_END wrap your header declarations.
// ================================================================

#ifdef __cplusplus
    #define EXTERN_C_BEGIN extern "C" {
    #define EXTERN_C_END   }
#else
    #define EXTERN_C_BEGIN
    #define EXTERN_C_END
#endif

// ================================================================
// 4. Static assertions (portable across C11 and C++11)
// ================================================================

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    // C11 or later
    #define STATIC_ASSERT(expr) _Static_assert(expr, #expr)
#elif defined(__cplusplus) && __cplusplus >= 201103L
    // C++11 or later
    #define STATIC_ASSERT(expr) static_assert(expr, #expr)
#else
    // Fallback for older compilers (causes a compile-time divide-by-zero on failure)
    #define STATIC_ASSERT(expr) typedef char static_assert_##__LINE__[(expr) ? 1 : -1]
#endif

// ================================================================
// 5. Null pointer definition (unifies nullptr / NULL)
// ================================================================

#ifndef NULL
    #define NULL ((void*)0)
#endif

#endif // COMPAT_C_H