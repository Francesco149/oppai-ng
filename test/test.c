#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../oppai.c"
#include "test_suite.c" /* defines suite */

#define ERROR_MARGIN 0.02f /* pp can be off by +- 2% */

/*
 * margin is actually
 * - 3x for < 100pp
 * - 2x for 100-200pp
 * - 1.5x for 200-300pp
 */

void print_score(score_t* s) {
  char mods_str_buf[20];
  char* mods_str = mods_str_buf;

  strcpy(mods_str, "nomod");

#define m(mod) \
  if (s->mods & MODS_##mod) { \
    mods_str += sprintf(mods_str, #mod); \
  } \

  m(HR) m(NC) m(HT) m(SO) m(NF) m(EZ) m(DT) m(FL) m(HD)
#undef m

  printf("m=%d %u +%s %dx %dx300 %dx100 %dx50 %dxmiss %g pp\n",
    s->mode, s->id, mods_str_buf, s->max_combo, s->n300, s->n100,
    s->n50, s->nmiss, s->pp);
}

int main() {
  char fname_buf[4096];
  char* fname = fname_buf;
  int i, n = (int)(sizeof(suite) / sizeof(suite[0]));
  float max_err[2] = { 0, 0 };
  int max_err_map[2] = { 0, 0 };
  float avg_err[2] = { 0, 0 };
  int count[2] = { 0, 0 };
  float error = 0;
  float error_percent = 0;
  ezpp_t ez = ezpp_new();
  int err;
  int last_id = 0;

  fname += sprintf(fname, "test_suite/");

  for (i = 0; i < n; ++i) {
    score_t* s = &suite[i];
    float margin;
    float pptotal;

    print_score(s);
    if (s->id != last_id) {
      sprintf(fname, "%u.osu", s->id);
      last_id = s->id;
      /* force reparse and don't reuse prev map ar/cs/od/hp */
      ezpp_set_base_cs(ez, -1);
      ezpp_set_base_ar(ez, -1);
      ezpp_set_base_od(ez, -1);
      ezpp_set_base_hp(ez, -1);
    }
    ezpp_set_mods(ez, s->mods);
    ezpp_set_accuracy(ez, s->n100, s->n50);
    ezpp_set_nmiss(ez, s->nmiss);
    ezpp_set_combo(ez, s->max_combo);
    ezpp_set_mode_override(ez, s->mode);
    err = ezpp(ez, fname_buf);
    if (err < 0) {
      printf("%s\n", errstr(err));
      exit(1);
    }
    pptotal = ezpp_pp(ez);
    ++count[s->mode];

    margin = (float)s->pp * ERROR_MARGIN;
    if (s->pp < 100) {
      margin *= 3;
    } else if (s->pp < 200) {
      margin *= 2;
    } else if (s->pp < 300) {
      margin *= 1.5f;
    }

    error = (float)fabs(pptotal - (float)s->pp);
    error_percent = error / (float)s->pp;
    avg_err[s->mode] += error_percent;
    if (error_percent > max_err[s->mode]) {
      max_err[s->mode] = error_percent;
      max_err_map[s->mode] = s->id;
    }

    if (error >= margin) {
      printf("failed test: got %g pp, expected %g\n", pptotal, s->pp);
      exit(1);
    }
  }

  for (i = 0; i < 2; ++i) {
    switch (i) {
      case MODE_STD: puts("osu"); break;
      case MODE_TAIKO: puts("taiko"); break;
    }
    avg_err[i] /= count[i];
    printf("%d scores\n", count[i]);
    printf("avg err %f\n", avg_err[i]);
    printf("max err %f on %d\n", max_err[i], max_err_map[i]);
  }

  ezpp_free(ez);

  return 0;
}

