#include <stdio.h>
#include <stdarg.h>

void f (int i, ...) {
  va_list args1, args2;
  va_start (args1, i);
  p_va_copy(args2, args1);
  if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42)
    exit (1);
  va_end (args1); va_end (args2);
}

int main() {
  f (0, 42);
  exit(0);
}
