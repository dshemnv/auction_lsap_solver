#ifndef _CONSTANTES_
#define _CONSTANTES_

#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define MAX_ITER 5000
#define MULT 65536
#define MIN_INF 65536

#define eps ((val_t)(0.0001 * MULT))

#ifndef __ROWS
#define __ROWS 16
#endif
#define __COLS __ROWS

#define SIZE __COLS *__ROWS

typedef int val_t;
typedef bool bool_t;

typedef int object_t;
typedef int object_cnt_t;

typedef int agent_t;
typedef int agent_cnt_t;

#endif
