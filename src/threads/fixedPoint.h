#include <stdint.h>

#define f (1<<14)


int convert_to_fixed(int n)
{
  return n * f;
}

int convert_to_int_floor(int x)
{
  return x / f;
}
int convert_to_int_round(int x)
{
  if (x >= 0)
  {
    return (x + f / 2) / f;
  }
  else
  {
    return (x - f / 2) / f;
  }
}

int add_two_fixed(int x, int y)
{
  return x + y;
}
int sub_two_fixed(int x, int y)
{
  return x - y;
}
int mult_two_fixed(int x, int y)
{
  return ((int64_t)x) * y / f;
}
int div_two_fixed(int x, int y)
{
  return ((int64_t)x) * f / y;
}


int add_int_to_fixed(int x, int n)
{
  return x + n * f;
}
int sub_int_from_fixed(int x, int n)
{
  return x - n * f;
}
int mult_fixed_by_int(int x, int n)
{
  return x * n;
}
int div_fixed_by_int(int x, int n)
{
  return x / n;
}