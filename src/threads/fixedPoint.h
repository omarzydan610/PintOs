#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

#define f (1 << 14)

typedef struct
{
  int value;
} fixed_point;

fixed_point convert_to_fixed(int n)
{
  fixed_point result;
  result.value = n * f;
  return result;
}

int int_floor(fixed_point x)
{
  return x.value / f;
}
int int_round(fixed_point x)
{
  if (x.value >= 0)
  {
    return (x.value + f / 2) / f;
  }
  else
  {
    return (x.value - f / 2) / f;
  }
}

fixed_point add_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = x.value + y.value;
  return result;
}
fixed_point sub_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = x.value - y.value;
  return result;
}
fixed_point mult_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = ((int64_t)x.value) * y.value / f;
  return result;
}
fixed_point div_two_fixed(fixed_point x, fixed_point y)
{
  fixed_point result;
  result.value = ((int64_t)x.value) * f / y.value;
  return result;
}

fixed_point add_int_to_fixed(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value + n * f;
  return result;
}
fixed_point sub_int_from_fixed(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value - n * f;
  return result;
}
fixed_point mult_fixed_by_int(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value * n;
  return result;
}
fixed_point div_fixed_by_int(fixed_point x, int n)
{
  fixed_point result;
  result.value = x.value / n;
  return result;
}

#endif /* THREADS_FIXED_POINT_H */