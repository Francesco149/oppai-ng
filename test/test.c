#define OPPAI_IMPLEMENTATION
#include "../oppai.c"
#include "test_suite.c" /* defines suite */

#define ERROR_MARGIN 0.02 /* pp can be off by +- 2% */

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

  info("%u +%s %dx %hdx300 %hdx100 %hdx50 %hdxmiss %g pp\n",
    s->id, mods_str_buf, s->max_combo, s->n300, s->n100,
    s->n50, s->nmiss, s->pp);
}

int main(int argc, char* argv[]) {
  char fname_buf[4096];
  char* fname = fname_buf;
  int i, n = (int)(sizeof(suite) / sizeof(suite[0]));
  float max_err = 0;
  int max_err_map = 0;
  float avg_err = 0;
  float error = 0;
  float error_percent = 0;
  ezpp_t ez = ezpp_new();
  int err;
  int last_id = 0;

  fname += sprintf(fname, "test_suite/");

  for (i = 0; i < n; ++i) {
    score_t* s = &suite[i];
    double margin;
    float pptotal;

    print_score(s);
    if (s->id != last_id) {
      sprintf(fname, "%u.osu", s->id);
      last_id = s->id;
      ezpp_set_base_cs(ez, 0); /* force reparse */
      ezpp_set_base_ar(ez, 0);
      ezpp_set_base_od(ez, 0);
      ezpp_set_base_hp(ez, 0);
    }
    ezpp_set_mods(ez, s->mods);
    ezpp_set_accuracy(ez, s->n100, s->n50);
    ezpp_set_nmiss(ez, s->nmiss);
    ezpp_set_combo(ez, s->max_combo);
    err = ezpp(ez, fname_buf);
    if (err < 0) {
      info("%s\n", errstr(err));
      exit(1);
    }
    pptotal = ezpp_pp(ez);

    margin = s->pp * ERROR_MARGIN;
    if (s->pp < 100) {
      margin *= 3;
    } else if (s->pp < 200) {
      margin *= 2;
    } else if (s->pp < 300) {
      margin *= 1.5;
    }

    error = fabs(pptotal - s->pp);
    error_percent = error / s->pp;
    avg_err += error_percent;
    if (error_percent > max_err) {
      max_err = error_percent;
      max_err_map = s->id;
    }

    if (error >= margin) {
      info("failed test: got %g pp, expected %g\n", pptotal, s->pp);
      exit(1);
    }
  }

  avg_err /= n;
  printf("avg err %f\n", avg_err);
  printf("max err %f on %d\n", max_err, max_err_map);
  ezpp_free(ez);

  return 0;
}

