#define OPPAI_IMPLEMENTATION
#include "oppai.c"

char buf[1000000];
int mods[] = { 0, MODS_HR, MODS_HD | MODS_HR, MODS_DT, MODS_HD | MODS_DT };
#define N_MODS (sizeof(mods) / sizeof(mods[0]))

void print_mods(int mods) {
  putchar('+');
  if (!mods) puts("nomod");
  else {
    if (mods & MODS_HD) printf("hd");
    if (mods & MODS_HR) printf("hr");
    if (mods & MODS_DT) printf("dt");
    puts("");
  }
}

int main(int argc, char* argv[]) {
  int i, j, n, acc;
  ezpp_t ez = ezpp_new();
  ezpp_set_autocalc(ez, 1);
  for (i = 1; i < argc; ++i) {
    FILE* f = fopen(argv[i], "r");
    n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    ezpp_data(ez, buf, n);
    printf("%s - %s [%s]\n", ezpp_artist(ez), ezpp_title(ez),
      ezpp_version(ez));
    for (j = 0; j < N_MODS; ++j) {
      print_mods(mods[j]);
      ezpp_set_mods(ez, mods[j]);
      for (acc = 95; acc <= 100; ++acc) {
        ezpp_set_accuracy_percent(ez, acc);
        printf("%d%% -> %gpp\n", acc, ezpp_pp(ez));
      }
    }
    puts("");
  }
  ezpp_free(ez);
  return 0;
}
