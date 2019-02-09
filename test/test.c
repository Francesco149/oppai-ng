#define OPPAI_IMPLEMENTATION
#include "../oppai.c"
#include "test_suite.c" /* defines suite */

#define ERROR_MARGIN 0.06 /* pp can be off by +- 6% */

/*
 * margin is actually
 * - 3x for < 100pp
 * - 2x for 100-200pp
 * - 1.5x for 200-300pp
 */

#ifndef OPPAI_NOCURL
#include <curl/curl.h>

size_t wrchunk(void* p, size_t cb, size_t nmemb, void* fd) {
  return fwrite(p, cb, nmemb, (FILE*)fd);
}

#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#define mkdir(x) mkdir(x, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#else
#include <direct.h>
#define mkdir _mkdir
#define unlink _unlink
#endif

#endif /* OPPAI_NOCURL */

void check_err(int result) {
  if (result < 0) {
    info("%s\n", errstr(result));
    exit(1);
  }
}

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

#ifdef OPPAI_DEBUG
void print_memory_usage(parser_t* pa, diff_calc_t* dc) {
  int arena = 0, timing = 0, objects = 0, strain = 0;
  if (pa) {
    arena = pa->arena.blocks.len * ARENA_BLOCK_SIZE;
    timing = pa->timing_points.len * sizeof(pa->timing_points.data[0]);
    objects = pa->objects.len * sizeof(pa->objects.data[0]);
  }
  if (dc) {
    strain = dc->highest_strains.len * sizeof(dc->highest_strains.data[0]);
  }
  info(
    "-------------------------\n"
    "arena: %db\n"
    "timing: %db\n"
    "objects: %db\n"
    "strains: %db\n"
    "total: %db\n"
    "-------------------------\n",
    arena, timing, objects, strain,
    arena + timing + objects + strain
  );
}
#else
#define print_memory_usage(x, y)
#endif /* OPPAI_DEBUG */

int main(int argc, char* argv[]) {
  char fname_buf[4096];
  char* fname = fname_buf;

  int i;
  int n = (int)(sizeof(suite) / sizeof(suite[0]));

  float max_err = 0;
  int max_err_map = 0;
  float avg_err = 0;
  float error = 0;
  float error_percent = 0;

#ifdef OPPAI_EZ
  ezpp_t ez;
#else
  int err;
  parser_t* pstate = 0;
  diff_calc_t stars;
  beatmap_t map;
  beatmap_stats_t mapstats;
  pp_params_t params;
  pp_calc_t pp;

#ifndef OPPAI_NOCURL
  CURL* curl = 0;
  char url_buf[128];
  char* url = url_buf;
#endif

  pstate = (parser_t*)malloc(sizeof(parser_t));
  if (!pstate) {
    check_err(ERR_OOM);
  }

  check_err(p_init(pstate));
  check_err(d_init(&stars));
#endif

  fname += sprintf(fname, "test_suite/");

  for (i = 0; i < n; ++i) {
    FILE* f;
    score_t* s = &suite[i];
    double margin;
    float pptotal;

    print_score(s);
    sprintf(fname, "%u.osu", s->id);
#ifdef OPPAI_EZ
    (void)f;
    ezpp_init(&ez);
    ez.mods = s->mods;
    ez.n300 = s->n300;
    ez.n100 = s->n100;
    ez.n50 = s->n50;
    ez.nmiss = s->nmiss;
    ez.combo = s->max_combo;
    check_err(ezpp(&ez, fname_buf));
    pptotal = ez.pp;
#else
#ifndef OPPAI_NOCURL
trycalc:
#endif
    f = fopen(fname_buf, "rb");
    memset(pstate, 0, sizeof(parser_t));
    err = p_map(pstate, &map, f);
    if (err < 0) {
#ifndef OPPAI_NOCURL
      /*
       * TODO: properly error check&log curl
       * also pull this out into a function maybe
       */

      CURLcode res;

      info("%s\n", errstr(err));
      if (f) {
        fclose(f);
        f = 0;
      }
      mkdir("test_suite");

      if (!curl) {
        puts("initializing curl");
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (!curl) {
          info("curl_easy_init failed\n");
          exit(1);
        }
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wrchunk);
        url += sprintf(url, "http://osu.ppy.sh/osu/");
      }

      sprintf(url, "%u", s->id);
      curl_easy_setopt(curl, CURLOPT_URL, url_buf);

      printf("downloading %s\n", url_buf);
      f = fopen(fname_buf, "wb");
      if (!f) {
        perror("fopen");
        exit(1);
      }

      curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
      res = curl_easy_perform(curl);
      fclose(f);
      f = 0;

      if (res != CURLE_OK) {
        info("curl_easy_perform failed");
        unlink(fname_buf);
        exit(1);
      }

      goto trycalc;
#else
      info("please download the test suite from "
        "http://www.hnng.moe/stuff/ and untar here, or "
        "recompile without OPPAI_NOCURL to automatically "
        "download it\n");
#endif /* OPPAI_NOCURL */
    }

    fclose(f);

    mapstats.ar = map.ar;
    mapstats.cs = map.cs;
    mapstats.od = map.od;
    mapstats.hp = map.hp;

    mods_apply(s->mods, &mapstats, APPLY_ALL);
    check_err(d_calc(&stars, &map, s->mods));
    pp_init(&params);

    params.aim = stars.aim;
    params.speed = stars.speed;
    params.mods = s->mods;
    params.n300 = s->n300;
    params.n100 = s->n100;
    params.n50 = s->n50;
    params.nmiss = s->nmiss;
    params.combo = s->max_combo;

    check_err(b_ppv2p(&map, &pp, &params));
    pptotal = pp.total;
#endif

    margin = s->pp * ERROR_MARGIN;
    if (s->pp < 100) {
      margin *= 3;
    }
    else if (s->pp < 200) {
      margin *= 2;
    }
    else if (s->pp < 300) {
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
#ifdef OPPAI_DEBUG
      int i;
#endif

      info("failed test: got %g pp, expected %g\n", pptotal, s->pp);

#if defined(OPPAI_DEBUG) && !defined(OPPAI_EZ)
      for (i = 0; i < map.nobjects; ++i) {
        object_t* o = &map.objects[i];

        printf("%gs [%g %g] ", o->time / 1000.0,
          o->strains[0], o->strains[1]);

        if (o->type & OBJ_CIRCLE) {
          printf("circle (%g, %g) (%g, %g)\n", o->pos[0], o->pos[1],
            o->normpos[0], o->normpos[1]);
        }
        else if (o->type & OBJ_SPINNER) {
          puts("spinner");
        }
        else if (o->type & OBJ_SLIDER) {
          printf("slider (%g, %g) (%g, %g)\n", o->pos[0], o->pos[1],
            o->normpos[0], o->normpos[1]);
        }
        else {
          printf("invalid hitobject %08X\n", o->type);
          break;
        }
      }

      printf("AR%g OD%g CS%g HP%g\n", mapstats.ar,
        mapstats.od, mapstats.cs, mapstats.hp);
      printf("%hu circles, %hu sliders, %hu spinners\n",
        map.ncircles, map.nsliders, map.nspinners);
      printf("%d/%dx\n", params.combo, params.max_combo);
      printf("%g stars (%g aim, %g speed)\n", stars.total,
        stars.aim, stars.speed);
      printf("%g%%\n", pp.accuracy * 100);
      printf("%g aim pp\n", pp.aim);
      printf("%g speed pp\n", pp.speed);
      printf("%g acc pp\n\n", pp.acc);
      printf("%g pp\n", pp.total);
#endif /* OPPAI_DEBUG */

      exit(1);
    }
  }

#ifndef OPPAI_EZ
  print_memory_usage(pstate, &stars);
  p_free(pstate);
  d_free(&stars);
  free(pstate);
#endif

  avg_err /= n;
  printf("avg err %f\n", avg_err);
  printf("max err %f on %d\n", max_err, max_err_map);

  return 0;
}

