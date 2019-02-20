/*
 * gcc reuse.c -lm
 * ./a.out /path/to/file.osu
 */

#define OPPAI_IMPLEMENTATION
#include "../oppai.c"

/*
 * for better performance, the same instance can be reused
 * settings are remembered and map is only reparsed if mods or cs change
 */

int main(int argc, char* argv[]) {
  ezpp_t ez = ezpp_new();
  ezpp_set_autocalc(ez, 1); /* autorecalc pp when changing any parameter */
  ezpp(ez, argv[1]);

  puts("---");
  puts("nomod fc");
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  puts("nomod 95% fc");
  ezpp_set_accuracy_percent(ez, 95);
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  puts("nomod 1x100 fc");
  ezpp_set_accuracy(ez, 1, 0);
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  puts("HD 1x100 1miss 300x");
  ezpp_set_mods(ez, MODS_HD);
  ezpp_set_nmiss(ez, 1);
  ezpp_set_combo(ez, 300);
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  puts("HDDT 1x100 1xmiss 300x");
  ezpp_set_mods(ez, MODS_HD | MODS_DT);
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  puts("HDDT 1x100 1xmiss 300x ends at object 300");
  ezpp_set_end(ez, 300);
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  puts("HDDT fc");
  ezpp_set_end(ez, 0);
  ezpp_set_combo(ez, -1);
  ezpp_set_accuracy(ez, 0, 0);
  ezpp_set_nmiss(ez, 0);
  printf("%gpp\n", ezpp_pp(ez));
  puts("---");

  ezpp_free(ez);
  return 0;
}
