#ifndef _MAIN_H
#define _MAIN_H

#include "iostream"
#include <cfloat>
#include <functional>
#include "math.h"

#define M_PI 3.14159265358979323846

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include <glad/glad.h>


/*
    core util
*/

#define LOGI(fmt, ...) do { printf("I: " fmt "\n",##__VA_ARGS__); } while (0)
#define LOGE(fmt, ...) do { printf("E: " fmt "\n",##__VA_ARGS__); } while (0)
#define THROW(fmt, ...) do { LOGE(__FILE__ ":%i " fmt, __LINE__, ##__VA_ARGS__); exit(1); } while (0)

template <typename T>
inline void vswap(T& a, T& b) { T t = a; a = b; b = t; }

typedef int32_t  S32;
typedef uint32_t U32;
typedef float   F32;
typedef uint8_t  U8;
typedef int8_t   S8;

#define F32_MAX 3.4e38

typedef U32 Inst;
#define NO_INSTANCE UINT32_MAX

/*
    error types
*/

typedef enum {
    OK = 0x00,
    ERROR = 0x01,
} Status;

typedef enum {
    AXIS_X = 0,
    AXIS_Y = 1
} Axis;

/*
    utilities
*/
template <typename T>
struct StaticBuffer {
    const T* data;
    U32 count;
    U32 bytes;

    // generate at compile time
    template <U32 N>
    constexpr StaticBuffer(const T (&arr)[N]) : data(arr), count(N), bytes(N * sizeof(T)) {};

    // generate at runtime
    StaticBuffer(const T* arr, const U32 N) : data(arr), count(N), bytes(N * sizeof(T)) {};
};

#endif