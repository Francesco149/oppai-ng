/*
 * gcc min.c -lm
 * cat /path/to/file.osu | ./a.out
 */

#define OPPAI_IMPLEMENTATION
#include "../oppai.c"

int main() {
  ezpp_t ez = ezpp_new();
  ezpp_set_mods(ez, MODS_HD | MODS_DT);
  ezpp(ez, "-");
  printf("%gpp\n", ezpp_pp(ez));
  return 0;
}

