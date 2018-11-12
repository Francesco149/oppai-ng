/*
 * gcc min.c -lm
 * cat /path/to/file.osu | ./a.out
 */

#define OPPAI_IMPLEMENTATION
#include "../oppai.c"

int main() {
  ezpp_t ez;
  ezpp_init(&ez);
  ez.mods = MODS_HD | MODS_DT;
  ezpp(&ez, "-");
  printf("%gpp\n", ez.pp);
  return 0;
}

