endian_test = r"""
#include <stdio.h>

int main(void)
{
  union endian_check {
    int i;
    char c;
  } endian;

  endian.i = 1;

  if (endian.c) {
    printf("L");
  } else {
    printf("B");
  }

  return 0;
}
"""
def CheckEndian(context):
  context.Message("Checking for Endian...")
  result = context.TryRun(endian_test, '.c')
  if result[0] == 1 and result[1] == "L":
    endian = 'Little'
  elif result[0] == 1 and result[1] == "B":
    endian = 'Big'
  else:
    endian = 'Unkown'
  context.Result(endian)
  return endian
