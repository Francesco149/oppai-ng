/*
 * gcc min.c -lm
 * cat /path/to/file.osu | ./a.out
 */

#define OPPAI_IMPLEMENTATION
#include "../oppai.c"

int main() {
  struct parser pstate;
  struct beatmap map;

  int mods;
  struct diff_calc stars;
  struct pp_calc pp;

  p_init(&pstate);
  p_map(&pstate, &map, stdin);

  mods = MODS_HD | MODS_DT;

  d_init(&stars);
  d_calc(&stars, &map, mods);
  printf("%g stars\n", stars.total);

  b_ppv2(&map, &pp, stars.aim, stars.speed, mods);
  printf("%gpp\n", pp.total);

  return 0;
}

