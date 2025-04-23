#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

#define F (1 << 14)

typedef struct
{
  int value;
} fixed_point;

static inline fixed_point convert_to_fixed(int n);
static inline int int_floor(fixed_point x);
static inline int int_round(fixed_point x);
static inline fixed_point add_two_fixed(fixed_point x, fixed_point y);
static inline fixed_point sub_two_fixed(fixed_point x, fixed_point y);
static inline fixed_point mult_two_fixed(fixed_point x, fixed_point y);
static inline fixed_point div_two_fixed(fixed_point x, fixed_point y);
static inline fixed_point add_int_to_fixed(fixed_point x, int n);
static inline fixed_point sub_int_from_fixed(fixed_point x, int n);
static inline fixed_point mult_fixed_by_int(fixed_point x, int n);
static inline fixed_point div_fixed_by_int(fixed_point x, int n);

static inline fixed_point convert_to_fixed(int n)
{
  fixed_point result;
  result.value = n * F;
  return result;
}

static inline int int_floor(fixed_point x)
{
  return x.value / F;
}

static inline int int_round(fixed_point x)
{
  if (x.value >= 0)
  {
    return (x.value + F / 2) / F;
  }
  else
  {
    return (x.value - F / 2) / F;
  }
}

static inline fixed_point add_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = x.value + y.value;
  return result;
}

static inline fixed_point sub_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = x.value - y.value;
  return result;
}

static inline fixed_point mult_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = ((int64_t)x.value) * y.value / F;
  return result;
}

static inline fixed_point div_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = ((int64_t)x.value) * F / y.value;
  return result;
}

static inline fixed_point add_int_to_fixed(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value + n * F;
  return result;
}

static inline fixed_point sub_int_from_fixed(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value - n * F;
  return result;
}

static inline fixed_point mult_fixed_by_int(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value * n;
  return result;
}

static inline fixed_point div_fixed_by_int(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value / n;
  return result;
}

#endif