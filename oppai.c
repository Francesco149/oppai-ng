/*
 * this is free and unencumbered software released into the
 * public domain.
 *
 * refer to the attached UNLICENSE or http://unlicense.org/
 * ------------------------------------------------------------------------
 * this is a pure C89 rewrite of oppai, my osu! difficulty and pp
 * calculator. it's meant to be tiny and easy to include in your projects
 * without pulling in dependencies.
 * ------------------------------------------------------------------------
 * usage:
 *
 * #define OPPAI_IMPLEMENTATION and include this file.
 * if multiple compilation units need to include oppai, only define
 * OPPAI_IMPLEMENTATION in one of them
 *
 * see the interface below this comment for detailed documentation
 * ------------------------------------------------------------------------
 * #define OPPAI_IMPLEMENTATION
 * #include "../oppai.c"
 *
 * int main() {
 *   ezpp_t ez;
 *   ezpp_init(&ez);
 *   ez.mods = MODS_HD | MODS_DT;
 *   ezpp(&ez, "-");
 *   printf("%gpp\n", ez.pp);
 *   return 0;
 * }
 * ------------------------------------------------------------------------
 * $ gcc test.c
 * $ cat /path/to/file.osu | ./a.out
 * ...
 */

#include <stdio.h>

#if defined(_WIN32) && !defined(OPPAI_IMPLEMENTATION)
#ifdef OPPAI_EXPORT
#define OPPAIAPI __declspec(dllexport)
#else
#define OPPAIAPI __declspec(dllimport)
#endif
#else
#define OPPAIAPI
#endif

#ifdef OPPAI_EXPORT
#define OPPAI_IMPLEMENTATION
#endif

OPPAIAPI void oppai_version(int* major, int* minor, int* patch);
OPPAIAPI char* oppai_version_str();

/* simple interface ---------------------------------------------------- */

struct ezpp;
typedef struct ezpp ezpp_t;

/* populate ezpp_t with default settings */
OPPAIAPI void ezpp_init(ezpp_t* ez);

/*
 * parse map and calculate difficulty and pp with advanced parameters,
 * see struct pp_params
 *
 * - if map is "-" the map is read from standard input
 * - if data_size is specified in ez, map is interpreted as raw beatmap
 *   data in memory
 */
OPPAIAPI int ezpp(ezpp_t* ez, char* map);

/*
 * - if data_size is  set, ezpp will interpret map as raw .osu file data
 * - mode defaults to MODE_STD
 * - mods default to MODS_NOMOD
 * - combo defaults to full combo
 * - nmiss defaults to 0
 * - score_version defaults to PP_DEFAULT_SCORING
 * - if accuracy_percent is set, n300/100/50 are automatically
 *   calculated and stored
 * - if n300/100/50 are set, accuracy_percent is automatically
 *   calculated and stored
 * - if none of the above are set, SS (100%) is assumed
 */

struct ezpp {
  /* inputs */
  int data_size;
  float ar_override, od_override, cs_override;
  int mode_override;
  int mode;
  int mods;
  int combo;
  int nmiss;
  int score_version;
  float accuracy_percent;
  int n300, n100, n50;
  int end; /* if set, the map will be cut to this object index */

  /* outputs */
  float stars;
  float aim_stars;
  float speed_stars;
  float pp, aim_pp, speed_pp, acc_pp;
};

/* errors -------------------------------------------------------------- */

/*
 * all functions that return int can return errors in the form
 * of a negative value. check if the return value is < 0 and call
 * errstr to get the error message
 */

#define ERR_MORE (-1)
#define ERR_SYNTAX (-2)
#define ERR_TRUNCATED (-3)
#define ERR_NOTIMPLEMENTED (-4)
#define ERR_IO (-5)
#define ERR_FORMAT (-6)
#define ERR_OOM (-7)

OPPAIAPI char* errstr(int err);

/* array --------------------------------------------------------------- */

/*
 * array_t(mytype) is a type-safe resizable array with mytype elements
 * you can use array_* macros to operate on it
 *
 * in case of out-of-memory, operations that can grow the array don't do
 * anything
 */

#define array_t(type) \
  struct { \
    int cap; \
    int len; \
    type* data; \
  }

#define array_reserve(arr, n) \
  array_reserve_i(n, array_unpack(arr))

#define array_free(arr) \
  array_free_i(array_unpack(arr))

#define array_alloc(arr) \
  (array_reserve((arr), (arr)->len + 1) \
    ? &(arr)->data[(arr)->len++] \
    : 0)

#define array_append(arr, x) \
  (array_reserve((arr), (arr)->len + 1) \
    ? ((arr)->data[(arr)->len++] = (x), 1) \
    : 0)

/* internal helpers, not to be used directly */
#define array_unpack(arr) \
  &(arr)->cap, \
  &(arr)->len, \
  (void**)&(arr)->data, \
  (int)sizeof((arr)->data[0])

OPPAIAPI int array_reserve_i(int n, int* cap, int* len, void** data,
  int esize);
OPPAIAPI void array_free_i(int* cap, int* len, void** data, int esize);

/* memory arena -------------------------------------------------------- */

/*
 * very simple allocator for when you want to allocate a bunch of stuff
 * and free it all at once. reduces malloc overhead by pre-allocating big
 * contiguous chunks of memory
 *
 * arena_t must be initialized to zero
 * arena_reserve and arena_alloc will return 0 on failure (out of memory)
 */

#define ARENA_ALIGN sizeof(void*)
#define ARENA_BLOCK_SIZE 4096

typedef struct {
  char* block;
  char* end_of_block;
  array_t(char*) blocks;
} arena_t;

/* ensures that there are at least min_size bytes reserved */
OPPAIAPI int arena_reserve(arena_t* arena, int min_size);
OPPAIAPI void* arena_alloc(arena_t* arena, int size);
OPPAIAPI char* arena_strndup(arena_t* m, char* s, int n);
OPPAIAPI void arena_free(arena_t* arena);

/* beatmap utils ------------------------------------------------------- */

/* object types used in struct object */
#define OBJ_CIRCLE (1<<0)
#define OBJ_SLIDER (1<<1)
#define OBJ_SPINNER (1<<3)

#define SOUND_NONE 0
#define SOUND_NORMAL (1<<0)
#define SOUND_WHISTLE (1<<1)
#define SOUND_FINISH (1<<2)
#define SOUND_CLAP (1<<3)

/* data about a single hitobject */
typedef struct object {
  float time; /* milliseconds */
  int type;

  /* only parsed for taiko maps */
  int nsound_types;
  int* sound_types;

  /* only used by d_calc */
  float normpos[2];
  float angle;
  float strains[2];
  int is_single; /* 1 if diff calc sees this as a singletap */
  float delta_time;
  float d_distance;

  float pos[2];
  float distance;  /* only for sliders */
  int repetitions;
} object_t;

/* timing point */
typedef struct timing {
  float time;        /* milliseconds */
  float ms_per_beat;
  int change;        /* if 0, ms_per_beat is -100.0f * sv_multiplier */
} timing_t;

#define MODE_STD 0
#define MODE_TAIKO 1

typedef struct beatmap {
  int format_version;
  int mode;
  int original_mode; /* the mode the beatmap was meant for */

  char* title;
  char* title_unicode;
  char* artist;
  char* artist_unicode;
  char* creator;
  char* version;

  int nobjects;
  object_t* objects;
  int ntiming_points;
  timing_t* timing_points;

  int ncircles, nsliders, nspinners;
  float hp, cs, od, ar, sv;
  float tick_rate;
} beatmap_t;

/* beatmap parser ------------------------------------------------------ */

/* non-null terminated string, used internally for parsing */
typedef struct slice {
  char* start;
  char* end; /* *(end - 1) is the last character */
} slice_t;

#define PARSER_OVERRIDE_MODE (1<<0) /* mode_override */
#define PARSER_FOUND_AR (1<<1)

/* beatmap parser's state */
typedef struct parser {
  int flags;
  int mode_override;

  /*
   * if a parsing error occurs last line and portion of the line
   * that was being parsed are stored in these two slices
   */
  slice_t lastpos;
  slice_t lastline;

  char buf[65536];  /* used to buffer data from the beatmap file */
  char section[64]; /* current section */

  /* internal allocators */
  arena_t arena;
  array_t(object_t) objects;
  array_t(timing_t) timing_points;

  beatmap_t* b;
} parser_t;

OPPAIAPI int p_init(parser_t* pa);
OPPAIAPI void p_free(parser_t* pa);

/*
 * parses a beatmap file and stores results in b.
 *
 * NOTE: b is valid only as long as pa is not deallocated or
 *   reused. if you need to store maps for longer than the
 *   parser's lifetime, you will have to manually copy.
 *
 * returns n. bytes processed on success, < 0 on failure
 */
OPPAIAPI int p_map(parser_t* pa, beatmap_t* b, FILE* f);
OPPAIAPI int p_map_mem(parser_t* pa, beatmap_t* b, char* data,
  int data_size);

/* mods utils ---------------------------------------------------------- */

#define MODS_NOMOD 0
#define MODS_NF (1<<0)
#define MODS_EZ (1<<1)
#define MODS_TD (1<<2)
#define MODS_HD (1<<3)
#define MODS_HR (1<<4)
#define MODS_SD (1<<5)
#define MODS_DT (1<<6)
#define MODS_RX (1<<7)
#define MODS_HT (1<<8)
#define MODS_NC (1<<9)
#define MODS_FL (1<<10)
#define MODS_AT (1<<11)
#define MODS_SO (1<<12)
#define MODS_AP (1<<13)
#define MODS_PF (1<<14)
#define MODS_KEY4 (1<<15) /* TODO: what are these abbreviated to? */
#define MODS_KEY5 (1<<16)
#define MODS_KEY6 (1<<17)
#define MODS_KEY7 (1<<18)
#define MODS_KEY8 (1<<19)
#define MODS_FADEIN (1<<20)
#define MODS_RANDOM (1<<21)
#define MODS_CINEMA (1<<22)
#define MODS_TARGET (1<<23)
#define MODS_KEY9 (1<<24)
#define MODS_KEYCOOP (1<<25)
#define MODS_KEY1 (1<<26)
#define MODS_KEY3 (1<<27)
#define MODS_KEY2 (1<<28)
#define MODS_SCOREV2 (1<<29)
#define MODS_TOUCH_DEVICE MODS_TD
#define MODS_NOVIDEO MODS_TD /* never forget */
#define MODS_SPEED_CHANGING (MODS_DT | MODS_HT | MODS_NC)
#define MODS_MAP_CHANGING (MODS_HR | MODS_EZ | MODS_SPEED_CHANGING)

/* beatmap stats after applying mods to them */
typedef struct beatmap_stats {
  float ar, od, cs, hp;
  float speed; /* multiplier */
  float odms;
} beatmap_stats_t;

/* flags bits for mods_apply */
#define APPLY_AR (1<<0)
#define APPLY_OD (1<<1)
#define APPLY_CS (1<<2)
#define APPLY_HP (1<<3)
#define APPLY_ALL (~0)

/*
 * calculates beatmap stats with mods applied.
 * * s should initially contain the base stats
 * * flags specifies which stats are touched
 * * initial speed will always be automatically set to 1
 *
 * returns 0 on success, or < 0 for errors
 *
 * example:
 *
 *    beatmap_stats_t s;
 *    s.ar = 9;
 *    mods_apply_m(MODE_STD, MODS_DT, &s, APPLY_AR);
 *    // s.ar is now 10.33f, s.speed is now 1.5f
 */
OPPAIAPI
int mods_apply_m(int mode, int mods, beatmap_stats_t* s, int flags);

/* legacy function, calls mods_apply(MODE_STD, mods, s, flags) */
OPPAIAPI void mods_apply(int mods, beatmap_stats_t* s, int flags);

/* diff calc ----------------------------------------------------------- */

/*
 * difficulty calculation state. just like with the parser, each
 * instance can be re-used in subsequent calls to d_calc
 */
typedef struct diff_calc {
  float speed_mul;
  float interval_end;
  float max_strain;
  array_t(float) highest_strains;
  beatmap_t* b;

  /*
   * set this to the milliseconds interval for the maximum bpm
   * you consider singletappable. defaults to 125 = 240 bpm 1/2
   * ((60000 / 240) / 2)
   */
  float singletap_threshold;

  /* calls to d_calc will store results here */
  float total;
  float aim;
  float aim_difficulty;
  float aim_length_bonus; /* unused for now */
  float speed;
  float speed_difficulty;
  float speed_length_bonus; /* unused for now */
  int nsingles;
  int nsingles_threshold;
} diff_calc_t;

OPPAIAPI int d_init(diff_calc_t* d);
OPPAIAPI void d_free(diff_calc_t* d);
OPPAIAPI int d_calc(diff_calc_t* d, beatmap_t* b, int mods);

/* pp calc ------------------------------------------------------------- */

typedef struct pp_calc {
  /* ppv2 will store results here */
  float total, aim, speed, acc;
  float accuracy; /* 0.0f - 1.0f */
} pp_calc_t;

/* default scoring system used by ppv2() and ppv2p() */
#define PP_DEFAULT_SCORING 1

/*
 * simplest possible call, calculates ppv2 for SS
 *
 * this also works for other modes by ignoring some parameters:
 * - taiko only uses pp, mode, speed, max_combo, base_od, mods
 */
OPPAIAPI
int ppv2(pp_calc_t* pp, int mode, float aim, float speed,
  float base_ar, float base_od, int max_combo, int nsliders, int ncircles,
  int nobjects, int mods);

/* simplest possible call for taiko ppv2 SS */
OPPAIAPI
int taiko_ppv2(pp_calc_t* pp, float speed, int max_combo,
  float base_od, int mods);

/* parameters for ppv2p */
typedef struct pp_params {
  /* required parameters */
  float aim, speed;
  float base_ar, base_od;
  int max_combo;
  int nsliders; /* required for scorev1 only */
  int ncircles; /* ^ */
  int nobjects;

  /* optional parameters */
  int mode;            /* defaults to MODE_STD */
  int mods;            /* defaults to MODS_NOMOD */
  int combo;           /* defaults to FC */
  int n300, n100, n50; /* defaults to SS */
  int nmiss;           /* defaults to 0 */
  int score_version;   /* defaults to PP_DEFAULT_SCORING */
} pp_params_t;

/*
 * initialize struct pp_params with the default values.
 * required values are left untouched
 */
OPPAIAPI void pp_init(pp_params_t* p);

/* calculate ppv2 with advanced parameters, see struct pp_params */
OPPAIAPI int ppv2p(pp_calc_t* pp, pp_params_t* p);

/*
 * same as ppv2p but fills params automatically with the map's
 * base_ar, base_od, max_combo, nsliders, ncircles, nobjects
 * so you only need to provide aim and speed
 */
OPPAIAPI int b_ppv2p(beatmap_t* map, pp_calc_t* pp, pp_params_t* p);

/* same as ppv2 but fills params like b_ppv2p */
OPPAIAPI
int b_ppv2(beatmap_t* map, pp_calc_t* pp, float aim, float speed,
  int mods);

/* --------------------------------------------------------------------- */

/* calculate accuracy (0.0f - 1.0f) */
OPPAIAPI float acc_calc(int n300, int n100, int n50, int misses);

/* calculate taiko accuracy (0.0f - 1.0f) */
OPPAIAPI float taiko_acc_calc(int n300, int n150, int nmisses);

/* round percent accuracy to closest amount of 300s, 100s, 50s */
OPPAIAPI
void acc_round(float acc_percent, int nobjects, int nmisses, int* n300,
  int* n100, int* n50);

/* round percent accuracy to closest amount of 300s and 150s (taiko) */
OPPAIAPI
void taiko_acc_round(float acc_percent, int nobjects, int nmisses,
  int* n300, int* n150);

/* --------------------------------------------------------------------- */

#define round_oppai(x) (float)floor((x) + 0.5f)
#define mymin(a, b) ((a) < (b) ? (a) : (b))
#define mymax(a, b) ((a) > (b) ? (a) : (b))
#define al_min mymin
#define al_max mymax

/* ##################################################################### */
/* ##################################################################### */
/* ##################################################################### */

#ifdef OPPAI_IMPLEMENTATION
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define OPPAI_VERSION_MAJOR 2
#define OPPAI_VERSION_MINOR 3
#define OPPAI_VERSION_PATCH 1
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define OPPAI_VERSION_STRING \
  STRINGIFY(OPPAI_VERSION_MAJOR) "." \
  STRINGIFY(OPPAI_VERSION_MINOR) "." \
  STRINGIFY(OPPAI_VERSION_PATCH)

OPPAIAPI void oppai_version(int* major, int* minor, int* patch) {
  *major = OPPAI_VERSION_MAJOR;
  *minor = OPPAI_VERSION_MINOR;
  *patch = OPPAI_VERSION_PATCH;
}

OPPAIAPI char* oppai_version_str() {
  return OPPAI_VERSION_STRING;
}

/* error utils --------------------------------------------------------- */

int info(char* fmt, ...) {
  int res;
  va_list va;
  va_start(va, fmt);
  res = vfprintf(stderr, fmt, va);
  va_end(va);
  return res;
}

OPPAIAPI
char* errstr(int err) {
  switch (err) {
    case ERR_MORE: return "call me again with more data";
    case ERR_SYNTAX: return "syntax error";
    case ERR_TRUNCATED:
      return "data was truncated, possibly because it was too big";
    case ERR_NOTIMPLEMENTED:
      return "requested a feature that isn't implemented";
    case ERR_IO: return "i/o error";
    case ERR_FORMAT: return "invalid input format";
    case ERR_OOM: return "out of memory";
  }
  info("W: got unknown error %d\n", err);
  return "unknown error";
}

/* math ---------------------------------------------------------------- */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float get_inf() {
  static unsigned raw = 0x7F800000;
  float* p = (float*)&raw;
  return *p;
}

float get_nan() {
  static unsigned raw = 0x7FFFFFFF;
  float* p = (float*)&raw;
  return *p;
}

/* dst = a - b */
void v2f_sub(float* dst, float* a, float* b) {
  dst[0] = a[0] - b[0];
  dst[1] = a[1] - b[1];
}

float v2f_len(float* v) {
  return (float)sqrt(v[0] * v[0] + v[1] * v[1]);
}

float v2f_dot(float* a, float* b) {
  return a[0] * b[0] + a[1] * b[1];
}

/* https://www.doc.ic.ac.uk/%7Eeedwards/compsys/float/nan.html */

int is_nan(float b) {
  int* p = (int*)&b;
  return (
    (*p > 0x7F800000 && *p < 0x80000000) ||
    (*p > 0x7FBFFFFF && *p <= 0xFFFFFFFF)
  );
}

/* string utils -------------------------------------------------------- */

int whitespace(char c) {
  switch (c) {
    case '\r':
    case '\n':
    case '\t':
    case ' ':
      return 1;
  }
  return 0;
}

int slice_write(slice_t* s, FILE* f) {
  return (int)fwrite(s->start, 1, s->end - s->start, f);
}

int slice_whitespace(slice_t* s) {
  char* p = s->start;
  for (; p < s->end; ++p) {
    if (!whitespace(*p)) {
      return 0;
    }
  }
  return 1;
}

/* trims leading and trailing whitespace */
void slice_trim(slice_t* s) {
  for (; s->start < s->end && whitespace(*s->start); ++s->start);
  for (; s->end > s->start && whitespace(*(s->end-1)); --s->end);
}

int slice_cmp(slice_t* s, char* str) {
  int len = (int)strlen(str);
  int s_len = (int)(s->end - s->start);
  if (len < s_len) {
    return -1;
  }
  if (len > s_len) {
    return 1;
  }
  return strncmp(s->start, str, len);
}

int slice_len(slice_t* s) {
  return (int)(s->end - s->start);
}

/*
 * splits s at any of the separators in separator_list and stores
 * pointers to the strings in arr.
 * returns the number of elements written to arr.
 * if more elements than nmax are found, err is set to
 * ERR_TRUNCATED
 */
int slice_split(slice_t* s, char* separator_list, slice_t* arr,
  int nmax, int* err)
{
  int res = 0;
  char* p = s->start;
  char* pprev = p;
  if (!nmax) {
    return 0;
  }
  if (!*separator_list) {
    *arr = *s;
    return 1;
  }
  for (; p <= s->end; ++p) {
    char* sep = separator_list;
    for (; *sep; ++sep) {
      if (p >= s->end || *sep == *p) {
        if (res >= nmax) {
          *err = ERR_TRUNCATED;
          goto exit;
        }
        arr[res].start = pprev;
        arr[res].end = p;
        pprev = p + 1;
        ++res;
        break;
      }
    }
  }
exit:
  return res;
}

/* array --------------------------------------------------------------- */

/*
 * these don't always use all params but we always pass all of them to
 * ensure that we get a compiler error on things that don't have the same
 * fields as an array struct
 */

OPPAIAPI
int array_reserve_i(int n, int* cap, int* len, void** data, int esize) {
  (void)len;
  if (*cap <= n) {
    void* newdata;
    int newcap = *cap ? *cap * 2 : 16;
    newdata = realloc(*data, esize * newcap);
    if (!newdata) {
      return 0;
    }
    *data = newdata;
    *cap = newcap;
  }
  return 1;
}

OPPAIAPI
void array_free_i(int* cap, int* len, void** data, int esize) {
  (void)esize;
  free(*data);
  *cap = 0;
  *len = 0;
  *data = 0;
}

/* memory arena -------------------------------------------------------- */

/* aligns x down to a power-of-two value a */
#define bit_align_down(x, a) \
  ((x) & ~((a) - 1))

/* aligns x up to a power-of-two value a */
#define bit_align_up(x, a) \
  bit_align_down((x) + (a) - 1, a)

OPPAIAPI
int arena_reserve(arena_t* arena, int min_size) {
  int size;
  char* new_block;
  if (arena->end_of_block - arena->block >= min_size) {
    return 1;
  }
  size = bit_align_up(al_max(min_size, ARENA_BLOCK_SIZE), ARENA_ALIGN);
  new_block = malloc(size);
  if (!new_block) {
    return 0;
  }
  arena->block = new_block;
  arena->end_of_block = new_block + size;
  array_append(&arena->blocks, arena->block);
  return 1;
}

OPPAIAPI
void* arena_alloc(arena_t* arena, int size) {
  void* res;
  if (!arena_reserve(arena, size)) {
    return 0;
  }
  size = bit_align_up(size, ARENA_ALIGN);
  res = arena->block;
  arena->block += size;
  return res;
}

OPPAIAPI
char* arena_strndup(arena_t* m, char* s, int n) {
  char* res = arena_alloc(m, n + 1);
  if (res) {
    memcpy(res, s, n);
    res[n] = 0;
  }
  return res;
}

OPPAIAPI
void arena_free(arena_t* arena) {
  int i;
  for (i = 0; i < arena->blocks.len; ++i) {
    free(arena->blocks.data[i]);
  }
  array_free(&arena->blocks);
  arena->block = 0;
  arena->end_of_block = 0;
}

/* mods ---------------------------------------------------------------- */

float od10_ms[] = { 20, 20 }; /* std, taiko */
float od0_ms[] = { 80, 50 };
#define AR0_MS 1800.0f
#define AR5_MS 1200.0f
#define AR10_MS 450.0f

float od_ms_step[] = { 6.0f, 3.0f };
#define AR_MS_STEP1 120.f /* ar0-5 */
#define AR_MS_STEP2 150.f /* ar5-10 */

OPPAIAPI
int mods_apply_m(int mode, int mods, beatmap_stats_t* s, int flags) {
  float od_ar_hp_multiplier;

  switch (mode) {
  case MODE_STD:
  case MODE_TAIKO:
    break;
  default:
    info("this gamemode is not yet supported for mods calc\n");
    return ERR_NOTIMPLEMENTED;
  }

  s->speed = 1;

  if (!(mods & MODS_MAP_CHANGING)) {
    int m = mode;
    if (flags & APPLY_OD) {
      s->odms = od0_ms[m] - (float)ceil(od_ms_step[m] * s->od);
    }
    return 0;
  }

  /* speed */
  if (mods & (MODS_DT | MODS_NC)) {
    s->speed *= 1.5f;
  }
  if (mods & MODS_HT) {
    s->speed *= 0.75f;
  }
  if (!flags) {
    return 0;
  }

  /* global multipliers */
  od_ar_hp_multiplier = 1;
  if (mods & MODS_HR) {
    od_ar_hp_multiplier *= 1.4f;
  }
  if (mods & MODS_EZ) {
    od_ar_hp_multiplier *= 0.5f;
  }

  /*
   * stats must be capped to 0-10 before HT/DT which brings them to a range
   * of -4.42f to 11.08f for OD and -5 to 11 for AR
   */

  /* od */
  if (flags & APPLY_OD) {
    int m = mode;
    s->od *= od_ar_hp_multiplier;
    s->odms = od0_ms[m] - (float)ceil(od_ms_step[m] * s->od);
    s->odms = mymin(od0_ms[m], mymax(od10_ms[m], s->odms));
    s->odms /= s->speed; /* apply speed-changing mods */
    s->od = (od0_ms[m] - s->odms) / od_ms_step[m]; /* back to stat */
  }

  /* ar */
  if (flags & APPLY_AR) {
    float arms;
    s->ar *= od_ar_hp_multiplier;

    /* convert AR into its milliseconds value */
    arms = s->ar <= 5
      ? (AR0_MS - AR_MS_STEP1 * (s->ar - 0))
      : (AR5_MS - AR_MS_STEP2 * (s->ar - 5));

    arms = mymin(AR0_MS, mymax(AR10_MS, arms));
    arms /= s->speed;
    s->ar = arms > AR5_MS
      ? (0 + (AR0_MS - arms) / AR_MS_STEP1)
      : (5 + (AR5_MS - arms) / AR_MS_STEP2);
  }

  /* cs */
  if (flags & APPLY_CS) {
    float cs_multiplier = 1;
    if (mods & MODS_HR) {
      cs_multiplier = 1.3f;
    }
    if (mods & MODS_EZ) {
      cs_multiplier = 0.5f;
    }
    s->cs *= cs_multiplier;
    s->cs = mymax(0.0f, mymin(10.0f, s->cs));
  }

  /* hp */
  if (flags & APPLY_HP) {
    s->hp = mymin(s->hp * od_ar_hp_multiplier, 10);
  }

  return 0;
}

OPPAIAPI
void mods_apply(int mods, beatmap_stats_t* s, int flags) {
  int n;
  n = mods_apply_m(MODE_STD, mods, s, flags);
  if (n < 0) {
    info("W: mods_apply failed: %s\n", errstr(n));
  }
}

/* beatmap ------------------------------------------------------------- */

/*
 * sliders get 2 + ticks combo (head, tail and ticks) each repetition adds
 * an extra combo and an extra set of ticks
 *
 * calculate the number of slider ticks for one repetition
 * ---
 * example: a 3.75f beats slider at 1x tick rate will go:
 * beat0 (head), beat1 (tick), beat2(tick), beat3(tick),
 * beat3.75f(tail)
 * so all we have to do is ceil the number of beats and subtract 1 to take
 * out the tail
 * ---
 * the -0.1f is there to prevent ceil from ceiling whole values like 1.0f to
 * 2.0f randomly
 */

OPPAIAPI
int b_max_combo(beatmap_t* b) {
  int res = b->nobjects;
  int i;

  float infinity = get_inf();
  float tnext = -infinity;
  int tindex = -1;

  float px_per_beat = infinity; /* for std sliders */

  /* taiko */
  float ms_per_beat = 0;          /* last timing change */
  float beat_len = infinity;      /* beat spacing */
  float duration = 0;             /* duration of the hit object */
  float tick_spacing = -infinity; /* slider tick spacing */

  if (!b->ntiming_points) {
    info("beatmap has no timing points\n");
    return ERR_FORMAT;
  }

  /* spinners don't give combo in taiko */
  if (b->mode == MODE_TAIKO) {
    res -= b->nspinners + b->nsliders;
  }

  /* slider ticks */
  for (i = 0; i < b->nobjects; ++i) {
    object_t* o = &b->objects[i];
    int ticks;
    float num_beats;

    if (!(o->type & OBJ_SLIDER)) {
      continue;
    }

    while (o->time >= tnext) {
      float sv_multiplier;
      timing_t* t;
      ++tindex;

      if (b->ntiming_points > tindex + 1) {
        tnext = b->timing_points[tindex + 1].time;
      } else {
        tnext = infinity;
      }

      t = &b->timing_points[tindex];

      sv_multiplier = 1.0f;
      if (!t->change && t->ms_per_beat < 0) {
        sv_multiplier = -100.0f / t->ms_per_beat;
      }

      switch (b->mode) {
      case MODE_STD:
        px_per_beat = b->sv * 100.0f * sv_multiplier;
        if (b->format_version < 8) {
          px_per_beat /= sv_multiplier;
        }
        break;

      case MODE_TAIKO: {
        /* see d_taiko for details on what this does */
        float velocity;

        if (b->original_mode == MODE_TAIKO) {
          /* no slider conversion for taiko -> taiko */
          continue;
        }

        if (t->change) {
          ms_per_beat = t->ms_per_beat;
        }

        beat_len = ms_per_beat;
        if (b->format_version < 8) {
          beat_len *= sv_multiplier;
        }

        velocity = 100.0f * b->sv / beat_len;
        duration = o->distance * o->repetitions / velocity;
        tick_spacing = mymin(beat_len / b->tick_rate,
            duration / o->repetitions);
        break;
      }

      default:
        return ERR_NOTIMPLEMENTED;
      }
    }

    if (b->mode == MODE_TAIKO) {
      if (tick_spacing > 0 && duration < 2 * beat_len) {
        res += (int)ceil((duration + tick_spacing / 8) / tick_spacing);
      }
      continue;
    }

    /* std slider ticks */
    num_beats = (o->distance * o->repetitions) / px_per_beat;

    ticks = (int)ceil((num_beats - 0.1f) / o->repetitions * b->tick_rate);
    --ticks;

    ticks *= o->repetitions;     /* account for repetitions */
    ticks += o->repetitions + 1; /* add heads and tails */

    /*
     * actually doesn't include first head because we already
     * added it by setting res = nobjects
     */
    res += mymax(0, ticks - 1);
  }

  return res;
}

/* beatmap parser ------------------------------------------------------ */

/* sets up parser for reuse. must have already been inited with p_init */
void p_reset(parser_t* pa, beatmap_t* b) {
  memset(pa->section, 0, sizeof(pa->section));
  memset(&pa->lastpos, 0, sizeof(pa->lastpos));
  memset(&pa->lastline, 0, sizeof(pa->lastline));
  pa->objects.len = 0;
  pa->timing_points.len = 0;

  /* TODO: reuse arena mem */
  arena_free(&pa->arena);

  pa->b = b;

  if (b) {
    memset(b, 0, sizeof(beatmap_t));
    b->ar = b->cs = b->hp = b->od = 5.0f;
    b->sv = b->tick_rate = 1.0f;
  }
}

OPPAIAPI
int p_init(parser_t* pa) {
  memset(pa, 0, sizeof(parser_t));
  p_reset(pa, 0);
  return 0;
}

OPPAIAPI
void p_free(parser_t* pa) {
  arena_free(&pa->arena);
  array_free(&pa->objects);
  array_free(&pa->timing_points);
}

/*
 * consume functions return the number of chars or < 0 on err
 * the destination slice is left untouched if there are errors
 *
 * NOTE: comments in beatmaps can only be an entire line because
 *   some properties such as author can contain //
 */

/* evil hack to set lastpos in one statement */
#define parse_err(e, lastpos_) \
  pa->lastpos = (lastpos_), \
  ERR_##e

int nop(int x) { return x; }

#define parse_warn(e, line) \
  info(e), info("\n"), print_line(line), nop(0)

/* consume until any of the characters in separators is found */
int consume_until(parser_t* pa, slice_t* s, char* separators,
  slice_t* dst)
{
  char* p = s->start;
  for (; p < s->end; ++p) {
    char* sep;
    for (sep = separators; *sep; ++sep) {
      if (*p == *sep) {
        dst->start = s->start;
        dst->end = p;
        return (int)(p - s->start);
      }
    }
  }
  return parse_err(MORE, *s);
}

/*
 * all parse_* functions expect s to be a single line and trimmed
 *
 * if the return type is int, they return n bytes consumed
 * if the return type is int, they will return zero on success
 *
 * on errors, parse_* functions return < 0 error codes
 */

#define print_line(line) \
  info("in line: "), \
  slice_write((line), stderr), \
  info("\n")

/* [name] */
int p_section_name(parser_t* pa, slice_t* s, slice_t* name) {
  int n;
  slice_t p = *s;
  if (*p.start++ != '[') {
    return parse_err(SYNTAX, p);
  }
  n = consume_until(pa, &p, "]", name);
  if (n < 0) {
    return n;
  }
  p.start += n;
  if (p.start != p.end - 1) { /* must end in ] */
    return parse_err(SYNTAX, p);
  }
  return (int)(p.start - s->start);
}

/* name: value (results are trimmed) */
int p_property(parser_t* pa, slice_t* s, slice_t* name, slice_t* value) {
  int n;
  char* p = s->start;
  n = consume_until(pa, s, ":", name);
  if (n < 0) {
    return n;
  }
  p += n;
  ++p; /* skip : */
  value->start = p;
  value->end = s->end;
  slice_trim(name);
  slice_trim(value);
  return (int)(s->end - s->start);
}

char* p_slicedup(parser_t* pa, slice_t* s) {
  return arena_strndup(&pa->arena, s->start, slice_len(s));
}

int p_metadata(parser_t* pa, slice_t* line) {
  slice_t name, value;
  beatmap_t* b = pa->b;
  int n = p_property(pa, line, &name, &value);
  if (n < 0) {
    return parse_warn("W: malformed metadata line", line);
  }
  if (!slice_cmp(&name, "Title")) {
    b->title = p_slicedup(pa, &value);
  }
  else if (!slice_cmp(&name, "TitleUnicode")) {
    b->title_unicode = p_slicedup(pa, &value);
  }
  else if (!slice_cmp(&name, "Artist")) {
    b->artist = p_slicedup(pa, &value);
  }
  else if (!slice_cmp(&name, "ArtistUnicode")) {
    b->artist_unicode = p_slicedup(pa, &value);
  }
  else if (!slice_cmp(&name, "Creator")) {
    b->creator = p_slicedup(pa, &value);
  }
  else if (!slice_cmp(&name, "Version")) {
    b->version = p_slicedup(pa, &value);
  }
  return n;
}

int p_general(parser_t* pa, slice_t* line) {
  beatmap_t* b = pa->b;
  slice_t name, value;
  int n;
  n = p_property(pa, line, &name, &value);
  if (n < 0) {
    return parse_warn("W: malformed general line", line);
  }

  if (!slice_cmp(&name, "Mode")) {
    if (sscanf(value.start, "%d", &b->original_mode) != 1){
      return parse_err(SYNTAX, value);
    }
    if (pa->flags & PARSER_OVERRIDE_MODE) {
      b->mode = pa->mode_override;
    } else {
      b->mode = b->original_mode;
    }
    switch (b->mode) {
    case MODE_STD:
    case MODE_TAIKO:
      break;
    default:
      return ERR_NOTIMPLEMENTED;
    }
  }

  return n;
}

float p_float(slice_t* value, int* success) {
  float res;
  char* p = value->start;
  if (*p == '-') {
    res = -1;
    ++p;
  } else {
    res = 1;
  }

  /* infinity symbol */
  if (!strncmp(p, "\xe2\x88\x9e", 3)) {
    res *= get_inf();
    *success = 1;
  } else {
    *success = sscanf(value->start, "%f", &res) == 1;
  }

  /* if it fails we can just use default values */
  return res;
}

int p_difficulty(parser_t* pa, slice_t* line) {
  float* dst = 0;
  slice_t name, value;
  int n = p_property(pa, line, &name, &value);
  if (n < 0) {
    return parse_warn("W: malformed difficulty line", line);
  }

  if (!slice_cmp(&name, "CircleSize")) {
    dst = &pa->b->cs;
  }
  else if (!slice_cmp(&name, "OverallDifficulty")) {
    dst = &pa->b->od;
  }
  else if (!slice_cmp(&name, "ApproachRate")) {
    dst = &pa->b->ar;
    pa->flags |= PARSER_FOUND_AR;
  }
  else if (!slice_cmp(&name, "HPDrainRate")) {
    dst = &pa->b->hp;
  }
  else if (!slice_cmp(&name, "SliderMultiplier")) {
    dst = &pa->b->sv;
  }
  else if (!slice_cmp(&name, "SliderTickRate")) {
    dst = &pa->b->tick_rate;
  }

  if (dst) {
    int success;
    *dst = p_float(&value, &success);
  }

  return n;
}


/*
 * time, ms_per_beat, time_signature_id, sample_set_id,
 * sample_bank_id, sample_volume, is_timing_change, effect_flags
 *
 * everything after ms_per_beat is optional
 */
int p_timing(parser_t* pa, slice_t* line) {
  int res = 0;
  int n, i;
  int err = 0;
  slice_t split[8];
  timing_t* t = array_alloc(&pa->timing_points);
  int success;

  if (!t) {
    return ERR_OOM;
  }

  t->change = 1;

  n = slice_split(line, ",", split, 8, &err);
  if (err < 0) {
    if (err == ERR_TRUNCATED) {
      info("W: timing point with trailing values");
      print_line(line);
    } else {
      return err;
    }
  }

  if (n < 2) {
    return parse_warn("W: malformed timing point", line);
  }

  res = (int)(split[n - 1].end - line->start);
  for (i = 0; i < n; ++i) {
    slice_trim(&split[i]);
  }

  t->time = p_float(&split[0], &success);
  if (!success) {
    return parse_warn("W: malformed timing point time", line);
  }

  t->ms_per_beat = p_float(&split[1], &success);
  if (!success) {
    return parse_warn("W: malformed timing point ms_per_beat",
      line);
  }

  if (n >= 7) {
    if (slice_len(&split[6]) < 1) {
      t->change = 1;
    } else {
      t->change = *split[6].start != '0';
    }
  }

  return res;
}

int p_objects(parser_t* pa, slice_t* line) {
  beatmap_t* b = pa->b;
  object_t* o = array_alloc(&pa->objects);
  int err = 0;
  int ne;
  slice_t e[11];
  int success;

  if (o) {
    memset(o, 0, sizeof(*o));
  } else {
    return ERR_OOM;
  }

  ne = slice_split(line, ",", e, 11, &err);
  if (err < 0) {
    if (err == ERR_TRUNCATED) {
      info("W: object with trailing values\n");
      print_line(line);
    } else {
      return err;
    }
  }

  if (ne < 5) {
    return parse_warn("W: malformed hitobject", line);
  }

  o->time = p_float(&e[2], &success);
  if (!success) {
    return parse_warn("W: malformed hitobject time", line);
  }

  if (sscanf(e[3].start, "%d", &o->type) != 1) {
    parse_warn("W: malformed hitobject type", line);
    o->type = OBJ_CIRCLE;
  }

  if (b->mode == MODE_TAIKO) {
    int* sound_type = arena_alloc(&pa->arena, sizeof(int));
    if (!sound_type) {
      return ERR_OOM;
    }
    if (sscanf(e[4].start, "%d", sound_type) != 1) {
      parse_warn("W: malformed hitobject sound type", line);
      *sound_type = SOUND_NORMAL;
    }
    o->nsound_types = 1;
    o->sound_types = sound_type;
    /* wastes 4 bytes when you have per-node sounds but w/e */
  }

  if (o->type & OBJ_CIRCLE) {
    ++b->ncircles;
    o->pos[0] = p_float(&e[0], &success);
    if (!success) {
      return parse_warn("W: malformed circle position", line);
    }
    o->pos[1] = p_float(&e[1], &success);
    if (!success) {
      return parse_warn("W: malformed circle position", line);
    }
  }

  /* ?,?,?,?,?,end_time,custom_sample_banks */
  else if (o->type & OBJ_SPINNER) {
    ++b->nspinners;
  }

  /*
   * x,y,time,type,sound_type,points,repetitions,distance,
   * per_node_sounds,per_node_samples,custom_sample_banks
   */
  else if (o->type & OBJ_SLIDER) {
    ++b->nsliders;
    if (ne < 7) {
      return parse_warn("W: malformed slider", line);
    }

    o->pos[0] = p_float(&e[0], &success);
    if (!success) {
      return parse_warn("W: malformed slider position", line);
    }

    o->pos[1] = p_float(&e[1], &success);
    if (!success) {
      return parse_warn("W: malformed slider position", line);
    }

    if (sscanf(e[6].start, "%d", &o->repetitions) != 1) {
      o->repetitions = 1;
      parse_warn("W: malformed slider repetitions", line);
    }

    if (ne > 7) {
      o->distance = p_float(&e[7], &success);
      if (!success) {
        parse_warn("W: malformed slider distance", line);
        o->distance = 0;
      }
    }

    /* per-node sound types */
    if (b->mode == MODE_TAIKO && ne > 8 && slice_len(&e[8]) > 0) {
      slice_t p = e[8];
      int i, nodes;

      /*
       * TODO: there's probably something subtly wrong with this.
       * sometimes we get less sound types than nodes
       * also I don't know if I'm supposed to include the previous
       * sound type from the single sound_type field
       */

      /* repeats + head and tail. no repeats is 1 repetition, so -1 */
      nodes = mymax(0, o->repetitions - 1) + 2;
      o->sound_types = arena_alloc(&pa->arena, sizeof(int) * nodes);
      if (!o->sound_types) {
        return ERR_OOM;
      }

      for (i = 0; i < nodes; ++i) {
        slice_t node;
        int n;
        int type;
        node.start = node.end = 0;
        n = consume_until(pa, &p, "|", &node);
        if (n < 0 && n != ERR_MORE) {
          pa->lastpos = p;
          return n;
        }
        if (node.start >= node.end || !node.start || p.start >= p.end) {
          break;
        }
        p.start += n + 1;
        if (sscanf(node.start, "%d", &type) != 1) {
          parse_warn("W: malformed sound type", line);
          type = SOUND_NORMAL;
        }
        o->sound_types[i] = type;
      }

      o->nsound_types = i;
    }
  }

  return (int)(e[ne - 1].end - line->start);
}

int p_line(parser_t* pa, slice_t* line) {
  int n = 0;

  if (line->start >= line->end) {
    /* empty line */
    return 0;
  }

  if (slice_whitespace(line)) {
    return (int)(line->end - line->start);
  }

  /* comments (according to lazer) */
  switch (*line->start) {
    case ' ':
    case '_':
      return (int)(line->end - line->start);
  }

  /* from here on we don't care about leading or trailing whitespace */
  slice_trim(line);
  pa->lastline = *line;

  /* C++ style comments */
  if (!strncmp(line->start, "//", 2)) {
    return 0;
  }

  /* new section */
  if (*line->start == '[') {
    slice_t section;
    int len;
    n = p_section_name(pa, line, &section);
    if (n < 0) {
      return n;
    }
    if (section.end - section.start >= sizeof(pa->section)) {
      parse_warn("W: truncated long section name", line);
    }
    len = (int)mymin(sizeof(pa->section) - 1, section.end - section.start);
    memcpy(pa->section, section.start, len);
    pa->section[len] = 0;
    return n;
  }

  if (!strcmp(pa->section, "Metadata")) {
    n = p_metadata(pa, line);
  }
  else if (!strcmp(pa->section, "General")) {
    n = p_general(pa, line);
  }
  else if (!strcmp(pa->section, "Difficulty")) {
    n = p_difficulty(pa, line);
  }
  else if (!strcmp(pa->section, "TimingPoints")) {
    n = p_timing(pa, line);
  }
  else if (!strcmp(pa->section, "HitObjects")) {
    n = p_objects(pa, line);
  } else {
    char* p = line->start;
    char* fmt_str = "file format v";
    for (; p < line->end && strncmp(p, fmt_str, 13); ++p);
    p += 13;
    if (p < line->end) {
      if (sscanf(p, "%d", &pa->b->format_version) == 1) {
        return (int)(line->end - line->start);
      }
    }
  }

  return n;
}

void p_begin(parser_t* pa, beatmap_t* b) {
  b->sv = b->tick_rate = 1;
  p_reset(pa, b);
}

void p_end(parser_t* pa, beatmap_t* b) {
  if (!(pa->flags & PARSER_FOUND_AR)) {
    /* in old maps ar = od */
    b->ar = b->od;
  }

  b->objects = pa->objects.data;
  b->nobjects = pa->objects.len;
  b->timing_points = pa->timing_points.data;
  b->ntiming_points = pa->timing_points.len;

  if (!b->title_unicode) {
    b->title_unicode = b->title;
  }

  if (!b->artist_unicode) {
    b->artist_unicode = b->artist;
  }

  #define s(x) b->x = b->x ? b->x : "(null)"

  s(title);
  s(title_unicode);
  s(artist);
  s(artist_unicode);
  s(creator);
  s(version);
}

OPPAIAPI
int p_map(parser_t* pa, beatmap_t* b, FILE* f) {
  int res = 0;
  char* pbuf;
  int bufsize;
  int n;
  int nread;

  p_begin(pa, b);

  if (!f) {
    return ERR_IO;
  }

  /* points to free space in the buffer */
  pbuf = pa->buf;

  /* reading loop */
  for (;;) {
    int nlines = 0; /* complete lines in the current chunk */
    slice_t s;      /* points to the remaining data in buf */
    int more_data;

    bufsize = (int)sizeof(pa->buf) - (int)(pbuf - pa->buf);
    nread = (int)fread(pbuf, 1, bufsize, f);
    if (!nread) {
      /* eof */
      break;
    }

    more_data = !feof(f);
    s.start = pa->buf;
    s.end = pbuf + nread;

    /* parsing loop */
    for (; s.start < s.end; ) {
      slice_t line;
      n = consume_until(pa, &s, "\n", &line);

      if (n < 0) {
        if (n != ERR_MORE) {
          return n;
        }
        if (!nlines) {
          /* line doesn't fit the entire buffer */
          return parse_err(TRUNCATED, s);
        }
        if (more_data) {
          /* we will finish reading this line later */
          break;
        }
        /* EOF, so we must process the remaining data as a line */
        line = s;
        n = (int)(s.end - s.start);
      } else {
        ++n; /* also skip the \n */
      }

      res += n;
      s.start += n;
      ++nlines;

      n = p_line(pa, &line);
      if (n < 0) {
        return n;
      }

      res += n;
    }

    /* done parsing what we read, prepare to read some more */

    /* move remaining data to the beginning of buf */
    memmove(pa->buf, s.start, s.end - s.start);

    /* adjust pbuf to point to free space */
    pbuf = pa->buf + (s.end - s.start);
  }

  p_end(pa, b);

  return res;
}

OPPAIAPI
int p_map_mem(parser_t* pa, beatmap_t* b, char* data,
  int data_size)
{
  int res = 0;
  int n;
  int nlines = 0; /* complete lines in the current chunk */
  slice_t s; /* points to the remaining data in buf */

  p_begin(pa, b);

  if (!data || data_size == 0) {
    return ERR_IO;
  }

  s.start = data;
  s.end = data + data_size;

  /* parsing loop */
  for (; s.start < s.end; ) {
    slice_t line;
    n = consume_until(pa, &s, "\n", &line);

    if (n < 0) {
      if (n != ERR_MORE) {
        return n;
      }
      if (!nlines) {
        /* line doesn't fit the entire buffer */
        return parse_err(TRUNCATED, s);
      }
      /* EOF, so we must process the remaining data as a line */
      line = s;
      n = (int)(s.end - s.start);
    } else {
      ++n; /* also skip the \n */
    }

    res += n;
    s.start += n;
    ++nlines;

    n = p_line(pa, &line);
    if (n < 0) {
      return n;
    }

    res += n;
  }

  p_end(pa, b);

  return res;
}

/* diff calc ----------------------------------------------------------- */

/* based on tom94's osu!tp aimod and osuElements */

#define DIFF_SPEED 0
#define DIFF_AIM 1

/* how much strains decay per interval */
float decay_base[] = { 0.3f, 0.15f };

/*
 * arbitrary thresholds to determine when a stream is spaced enough
 * that it becomes hard to alternate
 */
#define SINGLE_SPACING 125.0f

/* used to keep speed and aim balanced between eachother */
float weight_scaling[] = { 1400.0f, 26.25f };

/* non-normalized diameter where the circlesize buff starts */
#define CIRCLESIZE_BUFF_TRESHOLD 30.0f

#define STAR_SCALING_FACTOR 0.0675f /* star rating multiplier */

/*
 * 50% of the difference between aim and speed is added to star
 * rating to compensate aim only or speed only maps
 */
#define EXTREME_SCALING_FACTOR 0.5f

#define PLAYFIELD_WIDTH 512.0f /* in osu!pixels */
#define PLAYFIELD_HEIGHT 384.0f

/* spinners position */
float playfield_center[] = {
  PLAYFIELD_WIDTH / 2.0f, PLAYFIELD_HEIGHT / 2.0f
};

/*
 * strains are calculated by analyzing the map in chunks and then
 * taking the peak strains in each chunk.
 * this is the length of a strain interval in milliseconds.
 */
#define STRAIN_STEP 400.0f

/*
 * max strains are weighted from highest to lowest, and this is
 * how much the weight decays.
 */
#define DECAY_WEIGHT 0.9f

OPPAIAPI
int d_init(diff_calc_t* d) {
  memset(d, 0, sizeof(diff_calc_t));
  if (!array_reserve(&d->highest_strains, sizeof(float) * 600)) {
    return ERR_OOM;
  }
  d->singletap_threshold = 125; /* 240 bpm 1/2 */
  return 0;
}

OPPAIAPI
void d_free(diff_calc_t* d) {
  array_free(&d->highest_strains);
}
#define MAX_SPEED_BONUS 45.0f /* ~330BPM 1/4 streams */
#define MIN_SPEED_BONUS 75.0f /* ~200BPM 1/4 streams */
#define ANGLE_BONUS_SCALE 90
#define AIM_TIMING_THRESHOLD 107
#define SPEED_ANGLE_BONUS_BEGIN (5 * M_PI / 6)
#define AIM_ANGLE_BONUS_BEGIN (M_PI / 3)

/*
 * TODO: unbloat these params
 * this function has become a mess with the latest changes, I should split
 * it into separate funcs for speed and im
 */
float d_spacing_weight(float distance, float delta_time,
  float prev_distance, float prev_delta_time,
  float angle, int type, int* is_single)
{
  float angle_bonus;
  float strain_time = al_max(delta_time, 50.0f);
  switch (type) {
    case DIFF_SPEED: {
      float speed_bonus;
      *is_single = distance > SINGLE_SPACING;
      distance = al_min(distance, SINGLE_SPACING);
      delta_time = al_max(delta_time, MAX_SPEED_BONUS);
      speed_bonus = 1.0f;
      if (delta_time < MIN_SPEED_BONUS) {
        speed_bonus += (float)
          pow((MIN_SPEED_BONUS - delta_time) / 40.0f, 2);
      }
      angle_bonus = 1.0f;
      if (!is_nan(angle) && angle < SPEED_ANGLE_BONUS_BEGIN) {
        float s = (float)sin(1.5 * (SPEED_ANGLE_BONUS_BEGIN - angle));
        angle_bonus += (float)pow(s, 2) / 3.57f;
        if (angle < M_PI / 2) {
          angle_bonus = 1.28f;
          if (distance < ANGLE_BONUS_SCALE && angle < M_PI / 4) {
            angle_bonus += (1 - angle_bonus)
              * al_min((ANGLE_BONUS_SCALE - distance) / 10, 1);
          }
          else if (distance < ANGLE_BONUS_SCALE) {
            angle_bonus += (1 - angle_bonus)
              * al_min((ANGLE_BONUS_SCALE - distance) / 10, 1)
              * (float)sin((M_PI / 2 - angle) * 4 / M_PI);
          }
        }
      }
      return (
        (1 + (speed_bonus - 1) * 0.75f) *
        angle_bonus *
        (0.95f + speed_bonus * (float)pow(distance / SINGLE_SPACING, 3.5))
      ) / strain_time;
    }
    case DIFF_AIM: {
      float result = 0;
      float weighted_distance;
      float prev_strain_time = al_max(prev_delta_time, 50.0f);
      if (!is_nan(angle) && angle > AIM_ANGLE_BONUS_BEGIN) {
        angle_bonus = (float)sqrt(
          al_max(prev_distance - ANGLE_BONUS_SCALE, 0)
          * pow(sin(angle - AIM_ANGLE_BONUS_BEGIN), 2)
          * al_max(distance - ANGLE_BONUS_SCALE, 0)
        );
        result = 1.5f * (float)pow(al_max(0, angle_bonus), 0.99)
          / al_max(AIM_TIMING_THRESHOLD, prev_strain_time);
      }
      weighted_distance = (float)pow(distance, 0.99);
      return al_max(
        result + weighted_distance /
          al_max(AIM_TIMING_THRESHOLD, strain_time),
        weighted_distance / strain_time
      );
    }
  }
  return 0.0f;
}

void d_calc_strain(int type, object_t* o, object_t* prev, float speedmul) {
  float res = 0;
  float time_elapsed = (o->time - prev->time) / speedmul;
  float decay = (float)pow(decay_base[type], time_elapsed / 1000.0f);
  float scaling = weight_scaling[type];

  o->delta_time = time_elapsed;

  /* this implementation doesn't account for sliders */
  if (o->type & (OBJ_SLIDER | OBJ_CIRCLE)) {
    float diff[2];
    v2f_sub(diff, o->normpos, prev->normpos);
    o->d_distance = v2f_len(diff);
    res = d_spacing_weight(o->d_distance, time_elapsed, prev->d_distance,
      prev->delta_time, o->angle, type, &o->is_single);
    res *= scaling;
  }

  o->strains[type] = prev->strains[type] * decay + res;
}

int dbl_desc(void const* a, void const* b) {
  float x = *(float const*)a;
  float y = *(float const*)b;
  if (x < y) {
    return 1;
  }
  if (x == y) {
    return 0;
  }
  return -1;
}

int d_update_max_strains(diff_calc_t* d, float decay_factor,
  float cur_time, float prev_time, float cur_strain, float prev_strain,
  int first_obj)
{
  /* make previous peak strain decay until the current obj */
  while (cur_time > d->interval_end) {
    if (!array_append(&d->highest_strains, d->max_strain)) {
      return ERR_OOM;
    }
    if (first_obj) {
      d->max_strain = 0;
    } else {
      float decay;
      decay = (float)pow(decay_factor,
        (d->interval_end - prev_time) / 1000.0f);
      d->max_strain = prev_strain * decay;
    }
    d->interval_end += STRAIN_STEP * d->speed_mul;
  }

  d->max_strain = mymax(d->max_strain, cur_strain);
  return 0;
}

void d_weigh_strains2(diff_calc_t* d, float* pdiff, float* ptotal) {
  int i;
  int nstrains = 0;
  float* strains;
  float total = 0;
  float difficulty = 0;
  float weight = 1.0f;

  strains = (float*)d->highest_strains.data;
  nstrains = d->highest_strains.len;

  /* sort strains from highest to lowest */
  qsort(strains, nstrains, sizeof(float), dbl_desc);

  for (i = 0; i < nstrains; ++i) {
    total += (float)pow(strains[i], 1.2);
    difficulty += strains[i] * weight;
    weight *= DECAY_WEIGHT;
  }

  *pdiff = difficulty;
  if (ptotal) {
    *ptotal = total;
  }
}

float d_weigh_strains(diff_calc_t* d) {
  float diff;
  d_weigh_strains2(d, &diff, 0);
  return diff;
}

int d_calc_individual(int type, diff_calc_t* d) {
  int i;
  beatmap_t* b = d->b;

  /* 
   * the first object doesn't generate a strain,
   * so we begin with an incremented interval end
   */
  d->max_strain = 0.0f;
  d->interval_end = ceil(b->objects[0].time / (STRAIN_STEP * d->speed_mul))
    * (STRAIN_STEP * d->speed_mul);
  d->highest_strains.len = 0;

  for (i = 0; i < b->nobjects; ++i) {
    int err;
    object_t* o = &b->objects[i];
    object_t* prev = 0;
    float prev_time = 0, prev_strain = 0;
    if (i > 0) {
      prev = &b->objects[i - 1];
      d_calc_strain(type, o, prev, d->speed_mul);
      prev_time = prev->time;
      prev_strain = prev->strains[type];
    }
    err = d_update_max_strains(d, decay_base[type], o->time, prev_time,
      o->strains[type], prev_strain, i == 0);
    if (err < 0) {
      return err;
    }
  }

  /* 
   * the peak strain will not be saved for
   * the last section in the above loop
   */
  if (!array_append(&d->highest_strains, d->max_strain)) {
    return ERR_OOM;
  }

  switch (type) {
    case DIFF_SPEED:
      d_weigh_strains2(d, &d->speed, &d->speed_difficulty);
      break;
    case DIFF_AIM:
      d_weigh_strains2(d, &d->aim, &d->aim_difficulty);
      break;
  }
  return 0;
}

#define log10f (float)log10

float d_length_bonus(float stars, float difficulty) {
  return 0.32f + 0.5f * (log10f(difficulty + stars) - log10f(stars));
}

int d_std(diff_calc_t* d, int mods) {
  beatmap_t* b = d->b;
  int i;
  int res;
  float radius;
  float scaling_factor;
  beatmap_stats_t mapstats;

  /* apply mods and calculate circle radius at this CS */
  mapstats.cs = b->cs;
  mods_apply(mods, &mapstats, APPLY_CS);
  d->speed_mul = mapstats.speed;

  radius = (
    (PLAYFIELD_WIDTH / 16.0f) *
    (1.0f - 0.7f * ((float)mapstats.cs - 5.0f) / 5.0f)
  );

  /*
   * positions are normalized on circle radius so that we
   * can calc as if everything was the same circlesize
   */
  scaling_factor = 52.0f / radius;

  /* cs buff (originally from osuElements) */
  if (radius < CIRCLESIZE_BUFF_TRESHOLD) {
    scaling_factor *=
      1.0f + mymin((CIRCLESIZE_BUFF_TRESHOLD - radius), 5.0f) / 50.0f;
  }

  /* calculate normalized positions */
  for (i = 0; i < b->nobjects; ++i) {
    object_t* o = &b->objects[i];
    float* pos;
    float dot, det;
    if (o->type & OBJ_SPINNER) {
      pos = playfield_center;
    } else {
      /* sliders also begin with pos so it's fine */
      pos = o->pos;
    }
    o->normpos[0] = pos[0] * scaling_factor;
    o->normpos[1] = pos[1] * scaling_factor;
    if (i >= 2) {
      object_t* prev1 = &b->objects[i - 1];
      object_t* prev2 = &b->objects[i - 2];
      float v1[2], v2[2];
      v2f_sub(v1, prev2->normpos, prev1->normpos);
      v2f_sub(v2, o->normpos, prev1->normpos);
      dot = v2f_dot(v1, v2);
      det = v1[0] * v2[1] - v1[1] * v2[0];
      o->angle = (float)fabs(atan2(det, dot));
    } else {
      o->angle = get_nan();
    }
  }

  /* calculate speed and aim stars */
  res = d_calc_individual(DIFF_SPEED, d);
  if (res < 0) {
    return res;
  }

  res = d_calc_individual(DIFF_AIM, d);
  if (res < 0) {
    return res;
  }

  d->aim_length_bonus = d_length_bonus(d->aim, d->aim_difficulty);
  d->speed_length_bonus = d_length_bonus(d->speed, d->speed_difficulty);
  d->aim = (float)sqrt(d->aim) * STAR_SCALING_FACTOR;
  d->speed = (float)sqrt(d->speed) * STAR_SCALING_FACTOR;

  if (mods & MODS_TOUCH_DEVICE) {
    d->aim = (float)pow(d->aim, 0.8f);
  }

  /* calculate total star rating */
  d->total = d->aim + d->speed +
    (float)fabs(d->speed - d->aim) * EXTREME_SCALING_FACTOR;

  /* singletap stats */
  for (i = 1; i < b->nobjects; ++i) {
    object_t* o = &b->objects[i];
    if (o->is_single) {
      ++d->nsingles;
    }
    if (o->type & (OBJ_CIRCLE | OBJ_SLIDER)) {
      object_t* prev = &b->objects[i - 1];
      float interval = o->time - prev->time;
      interval /= mapstats.speed;
      if (interval >= d->singletap_threshold) {
        ++d->nsingles_threshold;
      }
    }
  }

  return 0;
}

/* taiko diff calc ----------------------------------------------------- */

#define TAIKO_STAR_SCALING_FACTOR 0.04125f
#define TAIKO_TYPE_CHANGE_BONUS 0.75f /* object type change bonus */
#define TAIKO_RHYTHM_CHANGE_BONUS 1.0f
#define TAIKO_RHYTHM_CHANGE_BASE_THRESHOLD 0.2f
#define TAIKO_RHYTHM_CHANGE_BASE 2.0f

typedef struct taiko_object {
  int hit;
  float strain;
  float time;
  float time_elapsed;
  int rim;
  int same_since; /* streak of hits of the same type (rim/center) */
  /*
   * was the last hit type change at an even same_since count?
   * -1 if there is no previous switch (for example if the
   * previous object was not a hit
   */
  int last_switch_even;
} taiko_object_t;

/* object type change bonus */
float taiko_change_bonus(taiko_object_t* cur, taiko_object_t* prev) {
  if (prev->rim != cur->rim) {
    cur->last_switch_even = prev->same_since % 2 == 0;

    if (prev->last_switch_even >= 0 &&
      prev->last_switch_even != cur->last_switch_even)
    {
      return TAIKO_TYPE_CHANGE_BONUS;
    }
  } else {
    cur->last_switch_even = prev->last_switch_even;
    cur->same_since = prev->same_since + 1;
  }

  return 0;
}

/* rhythm change bonus */
float taiko_rhythm_bonus(taiko_object_t* cur, taiko_object_t* prev) {
  float ratio;
  float diff;

  if (cur->time_elapsed == 0 || prev->time_elapsed == 0) {
    return 0;
  }

  ratio = mymax(prev->time_elapsed / cur->time_elapsed,
    cur->time_elapsed / prev->time_elapsed);

  if (ratio >= 8) {
    return 0;
  }

  /* this is log base TAIKO_RHYTHM_CHANGE_BASE of ratio */
  diff = (float)fmod(log(ratio) / log(TAIKO_RHYTHM_CHANGE_BASE), 1.0f);

  /*
   * threshold that determines whether the rhythm changed enough
   * to be worthy of the bonus
   */
  if (diff > TAIKO_RHYTHM_CHANGE_BASE_THRESHOLD &&
    diff < 1 - TAIKO_RHYTHM_CHANGE_BASE_THRESHOLD)
  {
    return TAIKO_RHYTHM_CHANGE_BONUS;
  }

  return 0;
}

void taiko_strain(taiko_object_t* cur, taiko_object_t* prev) {
  float decay;
  float addition = 1.0f;
  float factor = 1.0f;

  decay = (float)pow(decay_base[0], cur->time_elapsed / 1000.0f);

  /*
   * we only have strains for hits, also ignore objects that are
   * more than 1 second apart
   */
  if (prev->hit && cur->hit && cur->time - prev->time < 1000.0f) {
    addition += taiko_change_bonus(cur, prev);
    addition += taiko_rhythm_bonus(cur, prev);
  }

  /* 300+bpm streams nerf? */
  if (cur->time_elapsed < 50.0f) {
    factor = 0.4f + 0.6f * cur->time_elapsed / 50.0f;
  }

  cur->strain = prev->strain * decay + addition * factor;
}

void swap_ptrs(void** a, void** b) {
  void* tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

int d_taiko(diff_calc_t* d, int mods) {
  float infinity = get_inf();
  beatmap_t* b = d->b;
  int i;
  beatmap_stats_t mapstats;

  /* this way we can swap cur and prev without copying */
  taiko_object_t curprev[2];
  taiko_object_t* cur = &curprev[0];
  taiko_object_t* prev = &curprev[1];

  /*
   * these values keep track of the current timing point and
   * corresponding beat spacing. these are used to convert
   * sliders to taiko streams if they are suitable
   */

  float tnext = -infinity;        /* start time of next timing point */
  int tindex = -1;                /* timing point index */
  float ms_per_beat = 0;          /* last timing change */
  float beat_len = infinity;      /* beat spacing */
  float duration = 0;             /* duration of the hit object */
  float tick_spacing = -infinity; /* slider tick spacing */

  int result;

  if (!b->ntiming_points) {
    info("beatmap has no timing points\n");
    return ERR_FORMAT;
  }

  mods_apply(mods, &mapstats, 0);

  d->highest_strains.len = 0;
  d->max_strain = 0.0f;
  d->interval_end = STRAIN_STEP * mapstats.speed;
  d->speed_mul = mapstats.speed;

  /*
   * TODO: separate taiko conversion into its own function
   * so that it can be reused? probably slower, but cleaner,
   * more modular and more readable
   */
  for (i = 0; i < b->nobjects; ++i) {
    object_t* o = &b->objects[i];

    cur->hit = (o->type & OBJ_CIRCLE) != 0;
    cur->time = o->time;

    if (i > 0) {
      cur->time_elapsed = (cur->time - prev->time) / mapstats.speed;
    } else {
      cur->time_elapsed = infinity;
    }

    cur->strain = 1;
    cur->same_since = 1;
    cur->last_switch_even = -1;
    cur->rim = (o->sound_types[0] & (SOUND_CLAP|SOUND_WHISTLE)) != 0;

    if (b->original_mode == MODE_TAIKO) {
      goto continue_loop;
    }

    if (o->type & OBJ_SLIDER) {
      /* TODO: too much indentation, pull this out */
      int isound = 0;
      float j;

      while (o->time > tnext) {
        float sv_multiplier;
        float velocity;
        timing_t* t;

        ++tindex;
        if (b->ntiming_points > tindex + 1) {
          tnext = b->timing_points[tindex + 1].time;
        } else {
          tnext = infinity;
        }

        t = &b->timing_points[tindex];
        sv_multiplier = 1.0f;

        if (t->change) {
          ms_per_beat = t->ms_per_beat;
        }

        else if (t->ms_per_beat < 0) {
          sv_multiplier = -100.0f / t->ms_per_beat;
        }

        beat_len = ms_per_beat / sv_multiplier;
        velocity = 100.0f * b->sv / beat_len;

        /* format-specific quirk */
        if (b->format_version >= 8) {
          beat_len *= sv_multiplier;
        }

        /* this is similar to what we do in b_max_combo with px_per_beat */
        duration = o->distance * o->repetitions / velocity;

        /*
         * if slider is shorter than 1 beat, cut tick to exactly the length
         * of the slider
         */
        tick_spacing = mymin(beat_len / b->tick_rate,
            duration / o->repetitions);
      }

      /* drum roll, ignore */
      if (tick_spacing <= 0 || duration >= 2 * beat_len) {
        goto continue_loop;
      }

      /*
       * sliders that meet the requirements will
       * become streams of the slider's tick rate
       */
      for (j = o->time; j < o->time + duration + tick_spacing / 8;
         j += tick_spacing)
      {
        int sound_type = o->sound_types[isound];
        cur->rim = (sound_type & (SOUND_CLAP | SOUND_WHISTLE));
        cur->hit = 1;
        cur->time = j;

        cur->time_elapsed = (cur->time - prev->time) / mapstats.speed;
        cur->strain = 1;
        cur->same_since = 1;
        cur->last_switch_even = -1;

        /* update strains for this hit */
        if (i > 0 || j > o->time) {
          taiko_strain(cur, prev);
        }

        result = d_update_max_strains(d, decay_base[0], cur->time,
          prev->time, cur->strain, prev->strain, i == 0 && j == o->time);
        /* warning: j check might fail, floatcheck this */

        if (result < 0) {
          return result;
        }

        /* loop through the slider's sounds */
        ++isound;
        isound %= o->nsound_types;

        swap_ptrs((void**)&prev, (void**)&cur);
      }

      /*
       * since we processed the slider as multiple hits,
       * we must skip the prev/cur swap which we already did
       * in the above loop
       */
      continue;
    }

continue_loop:
    /* update strains for hits and other object types */
    if (i > 0) {
      taiko_strain(cur, prev);
    }

    result = d_update_max_strains(d, decay_base[0], cur->time, prev->time,
      cur->strain, prev->strain, i == 0);

    if (result < 0) {
      return result;
    }

    swap_ptrs((void**)&prev, (void**)&cur);
  }

  d->total =
  d->speed = d_weigh_strains(d) * TAIKO_STAR_SCALING_FACTOR;

  return 0;
}

/* --------------------------------------------------------------------- */

OPPAIAPI
int d_calc(diff_calc_t* d, beatmap_t* b, int mods) {
  d->b = b;
  switch (b->mode) {
  case MODE_STD:
    return d_std(d, mods);
  case MODE_TAIKO:
    return d_taiko(d, mods);
  }
  info("this gamemode is not yet supported\n");
  return ERR_NOTIMPLEMENTED;
}

/* acc calc ------------------------------------------------------------ */

OPPAIAPI
float acc_calc(int n300, int n100, int n50, int misses) {
  int total_hits = n300 + n100 + n50 + misses;
  float acc = 0;
  if (total_hits > 0) {
    acc = (n50 * 50.0f + n100 * 100.0f + n300 * 300.0f)
      / (total_hits * 300.0f);
  }
  return acc;
}

OPPAIAPI
void acc_round(float acc_percent, int nobjects, int misses, int* n300,
  int* n100, int* n50)
{
  int max300;
  float maxacc;
  misses = mymin(nobjects, misses);
  max300 = nobjects - misses;
  maxacc = acc_calc(max300, 0, 0, misses) * 100.0f;
  acc_percent = mymax(0.0f, mymin(maxacc, acc_percent));
  *n50 = 0;

  /* just some black magic maths from wolfram alpha */
  *n100 = (int)round_oppai(
    -3.0f * ((acc_percent * 0.01f - 1.0f) * nobjects + misses) * 0.5f
  );

  if (*n100 > nobjects - misses) {
    /* acc lower than all 100s, use 50s */
    *n100 = 0;
    *n50 = (int)round_oppai(
      -6.0f * ((acc_percent * 0.01f - 1.0f) * nobjects + misses) * 0.2f
    );
    *n50 = mymin(max300, *n50);
  } else {
    *n100 = mymin(max300, *n100);
  }

  *n300 = nobjects - *n100 - *n50 - misses;
}

OPPAIAPI
float taiko_acc_calc(int n300, int n150, int nmiss) {
  int total_hits = n300 + n150 + nmiss;
  float acc = 0;
  if (total_hits > 0) {
    acc = (n150 * 150.0f + n300 * 300.0f) / (total_hits * 300.0f);
  }
  return acc;
}

OPPAIAPI
void taiko_acc_round(float acc_percent, int nobjects, int nmisses,
  int* n300, int* n150)
{
  int max300;
  float maxacc;
  nmisses = mymin(nobjects, nmisses);
  max300 = nobjects - nmisses;
  maxacc = acc_calc(max300, 0, 0, nmisses) * 100.0f;
  acc_percent = mymax(0.0f, mymin(maxacc, acc_percent));
  /* just some black magic maths from wolfram alpha */
  *n150 = (int)round_oppai(
    -2.0f * ((acc_percent * 0.01f - 1.0f) * nobjects + nmisses)
  );
  *n150 = mymin(max300, *n150);
  *n300 = nobjects - *n150 - nmisses;
}

/* std pp calc --------------------------------------------------------- */

/* some kind of formula to get a base pp value from stars */
float base_pp(float stars) {
  return (float)pow(5.0f * mymax(1.0f, stars / 0.0675f) - 4.0f, 3.0f)
    / 100000.0f;
}

int ppv2x(pp_calc_t* pp, float aim, float speed, float base_ar,
  float base_od, int max_combo, int nsliders, int ncircles, int nobjects,
  int mods, int combo, int n300, int n100, int n50, int nmiss,
  int score_version)
{
  int nspinners = nobjects - nsliders - ncircles;
  beatmap_stats_t mapstats;

  /* various pp calc multipliers */
  float nobjects_over_2k = nobjects / 2000.0f;
  float length_bonus = (
    0.95f +
    0.4f * mymin(1.0f, nobjects_over_2k) +
    (nobjects > 2000 ? (float)log10(nobjects_over_2k) * 0.5f : 0.0f)
  );
  float miss_penality = (float)pow(0.97f, nmiss);
  float combo_break = (
    (float)pow(combo, 0.8f) / (float)pow(max_combo, 0.8f)
  );
  float ar_bonus;
  float final_multiplier;
  float acc_bonus, od_bonus;
  float od_squared;
  float hd_bonus;

  /* acc used for pp is different in scorev1 because it ignores sliders */
  float real_acc;

  memset(pp, 0, sizeof(pp_calc_t));

  /* sanitize some input */
  if (max_combo <= 0) {
    info("W: max_combo <= 0, changing to 1\n");
    max_combo = 1;
  }

  /* accuracy */
  pp->accuracy = acc_calc(n300, n100, n50, nmiss);

  switch (score_version) {
    case 1:
      /*
       * scorev1 ignores sliders since they are free 300s
       * apparently it also ignores spinners...
       * can go negative if we miss everything
       */
      real_acc = acc_calc(mymax(0, (int)n300 - nsliders - nspinners),
        n100, n50, nmiss);
      break;
    case 2:
      real_acc = pp->accuracy;
      ncircles = nobjects;
      break;
    default:
      info("unsupported scorev%d\n", score_version);
      return ERR_NOTIMPLEMENTED;
  }

  /* calculate stats with mods */
  mapstats.ar = base_ar;
  mapstats.od = base_od;

  mods_apply(mods, &mapstats, APPLY_AR | APPLY_OD);

  /* ar bonus -------------------------------------------------------- */
  ar_bonus = 1.0f;

  /* high ar bonus */
  if (mapstats.ar > 10.33f) {
    ar_bonus += 0.3f * (mapstats.ar - 10.33f);
  }

  /* low ar bonus */
  else if (mapstats.ar < 8.0f) {
    ar_bonus += 0.01f * (8.0f - mapstats.ar);
  }

  /* aim pp ---------------------------------------------------------- */
  pp->aim = base_pp(aim);
  pp->aim *= length_bonus;
  pp->aim *= miss_penality;
  pp->aim *= combo_break;
  pp->aim *= ar_bonus;

  /* hidden */
  hd_bonus = 1.0f;
  if (mods & MODS_HD) {
    hd_bonus += 0.04f * (12.0f - mapstats.ar);
  }

  pp->aim *= hd_bonus;

  /* flashlight */
  if (mods & MODS_FL) {
    float fl_bonus = 1.0f + 0.35f * mymin(1.0f, nobjects / 200.0f);
    if (nobjects > 200) {
      fl_bonus += 0.3f * mymin(1, (nobjects - 200) / 300.0f);
    }
    if (nobjects > 500) {
      fl_bonus += (nobjects - 500) / 1200.0f;
    }
    pp->aim *= fl_bonus;
  }

  /* acc bonus (bad aim can lead to bad acc) */
  acc_bonus = 0.5f + pp->accuracy / 2.0f;

  /* od bonus (high od requires better aim timing to acc) */
  od_squared = (float)pow(mapstats.od, 2);
  od_bonus = 0.98f + od_squared / 2500.0f;

  pp->aim *= acc_bonus;
  pp->aim *= od_bonus;

  /* speed pp -------------------------------------------------------- */
  pp->speed = base_pp(speed);
  pp->speed *= length_bonus;
  pp->speed *= miss_penality;
  pp->speed *= combo_break;
  if (mapstats.ar > 10.33f) {
    pp->speed *= ar_bonus;
  }
  pp->speed *= hd_bonus;

  /* "scale the speed value with accuracy slightly" */
  pp->speed *= 0.02f + pp->accuracy;

  /* "it is important to also consider accuracy difficulty when doing that" */
  pp->speed *= 0.96f + (od_squared / 1600.0f);

  /* acc pp ---------------------------------------------------------- */
  /* arbitrary values tom crafted out of trial and error */
  pp->acc = (float)pow(1.52163f, mapstats.od) *
    (float)pow(real_acc, 24.0f) * 2.83f;

  /* length bonus (not the same as speed/aim length bonus) */
  pp->acc *= mymin(1.15f, (float)pow(ncircles / 1000.0f, 0.3f));

  /* hidden bonus */
  if (mods & MODS_HD) {
    pp->acc *= 1.08f;
  }

  /* flashlight bonus */
  if (mods & MODS_FL) {
    pp->acc *= 1.02f;
  }

  /* total pp -------------------------------------------------------- */
  final_multiplier = 1.12f;

  /* nofail */
  if (mods & MODS_NF) {
    final_multiplier *= 0.90f;
  }

  /* spun-out */
  if (mods & MODS_SO) {
    final_multiplier *= 0.95f;
  }

  pp->total = (float)(
    pow(
      pow(pp->aim, 1.1f) +
      pow(pp->speed, 1.1f) +
      pow(pp->acc, 1.1f),
      1.0f / 1.1f
    ) * final_multiplier
  );

  return 0;
}

/* taiko pp calc ------------------------------------------------------- */

int taiko_ppv2x(pp_calc_t* pp, float stars, int max_combo,
  float base_od, int n150, int nmiss, int mods)
{
  beatmap_stats_t mapstats;
  int n300 = mymax(0, max_combo - n150 - nmiss);
  int result;
  float length_bonus;
  float final_multiplier;

  /* calculate stats with mods */
  mapstats.od = base_od;
  result = mods_apply_m(MODE_TAIKO, mods, &mapstats, APPLY_OD);
  if (result < 0) {
    return result;
  }

  pp->accuracy = taiko_acc_calc(n300, n150, nmiss);

  /* base acc pp */
  pp->acc = (float)pow(150.0f / mapstats.odms, 1.1f);
  pp->acc *= (float)pow(pp->accuracy, 15.0f) * 22.0f;

  /* length bonus */
  pp->acc *= mymin(1.15f, (float)pow(max_combo / 1500.0f, 0.3f));

  /* base speed pp */
  pp->speed = (float)pow(5.0f * mymax(1.0f, stars / 0.0075f) - 4.0f, 2.0f);
  pp->speed /= 100000.0f;

  /* length bonus (not the same as acc length bonus) */
  length_bonus = 1.0f + 0.1f * mymin(1.0f, max_combo / 1500.0f);
  pp->speed *= length_bonus;

  /* miss penality */
  pp->speed *= (float)pow(0.985f, nmiss);

#if 0
  /* combo scaling (removed?) */
  if (max_combo > 0) {
    pp->speed *=
      mymin(pow(max_combo - nmiss, 0.5f) / pow(max_combo, 0.5f), 1.0f);
  }
#endif

  /* speed mod bonuses */
  if (mods & MODS_HD) {
    pp->speed *= 1.025f;
  }

  if (mods & MODS_FL) {
    pp->speed *= 1.05f * length_bonus;
  }

  /* acc scaling */
  pp->speed *= pp->accuracy;

  /* overall mod bonuses */
  final_multiplier = 1.1f;

  if (mods & MODS_NF) {
    final_multiplier *= 0.90f;
  }

  if (mods & MODS_HD) {
    final_multiplier *= 1.10f;
  }

  pp->total = (
    (float)pow(
      pow(pp->speed, 1.1f) +
      pow(pp->acc, 1.1f),
      1.0f / 1.1f
    ) * final_multiplier
  );
  return 0;
}

OPPAIAPI
int taiko_ppv2(pp_calc_t* pp, float speed, int max_combo,
  float base_od, int mods)
{
  return taiko_ppv2x(pp, speed, max_combo, base_od, 0, 0, mods);
}

/* common pp calc stuff ------------------------------------------------ */

OPPAIAPI
void pp_init(pp_params_t* p) {
  p->mode = MODE_STD;
  p->mods = MODS_NOMOD;
  p->combo = -1;
  p->n300 = 0xFFFF;
  p->n100 = p->n50 = p->nmiss = 0;
  p->score_version = PP_DEFAULT_SCORING;
}

/* should be called inside ppv2p before calling ppv2x */
void pp_handle_default_params(pp_params_t* p) {
  if (p->combo < 0) {
    p->combo = p->max_combo - p->nmiss;
  }
  if (p->n300 == 0xFFFF) {
    p->n300 = p->nobjects - p->n100 - p->n50 - p->nmiss;
  }
}

OPPAIAPI
int ppv2(pp_calc_t* pp, int mode, float aim, float speed, float base_ar,
  float base_od, int max_combo, int nsliders, int ncircles, int nobjects,
  int mods)
{
  pp_params_t params;
  pp_init(&params);
  params.mode = mode;
  params.aim = aim, params.speed = speed;
  params.base_ar = base_ar;
  params.base_od = base_od;
  params.max_combo = max_combo;
  params.nsliders = nsliders;
  params.ncircles = ncircles;
  params.nobjects = nobjects;
  params.mods = mods;
  return ppv2p(pp, &params);
}

/* TODO: replace ppv2x with this? */
OPPAIAPI
int ppv2p(pp_calc_t* pp, pp_params_t* p) {
  pp_handle_default_params(p);
  switch (p->mode) {
  case MODE_STD:
    return ppv2x(pp, p->aim, p->speed, p->base_ar, p->base_od,
      p->max_combo, p->nsliders, p->ncircles, p->nobjects, p->mods,
      p->combo, p->n300, p->n100, p->n50, p->nmiss, p->score_version);
  case MODE_TAIKO:
    return taiko_ppv2x(pp, p->speed, p->max_combo, p->base_od,
      p->n100, p->nmiss, p->mods);
  }
  info("this mode is not yet supported for ppv2p\n");
  return ERR_NOTIMPLEMENTED;
}

OPPAIAPI
int b_ppv2(beatmap_t* b, pp_calc_t* pp, float aim, float speed, int mods) {
  pp_params_t params;
  int max_combo = b_max_combo(b);
  if (max_combo < 0) {
    return max_combo;
  }
  pp_init(&params);
  params.mode = b->mode;
  params.aim = aim, params.speed = speed;
  params.base_ar = b->ar;
  params.base_od = b->od;
  params.max_combo = max_combo;
  params.nsliders = b->nsliders;
  params.ncircles = b->ncircles;
  params.nobjects = b->nobjects;
  params.mods = mods;
  return ppv2p(pp, &params);
}

OPPAIAPI
int b_ppv2p(beatmap_t* map, pp_calc_t* pp, pp_params_t* p) {
  p->base_ar = map->ar;
  p->base_od = map->od;
  p->max_combo = b_max_combo(map);
  if (p->max_combo < 0) {
    return p->max_combo;
  }
  p->nsliders = map->nsliders;
  p->ncircles = map->ncircles;
  p->nobjects = map->nobjects;
  p->mode = map->mode;
  pp_handle_default_params(p);
  return ppv2p(pp, p);
}

OPPAIAPI void ezpp_init(ezpp_t* ez) {
  memset(ez, 0, sizeof(ezpp_t));
  ez->mode = MODE_STD;
  ez->mods = MODS_NOMOD;
  ez->combo = -1;
  ez->n300 = 0xFFFF;
  ez->n100 = ez->n50 = ez->nmiss = 0;
  ez->score_version = PP_DEFAULT_SCORING;
}

/* simple interface ---------------------------------------------------- */

OPPAIAPI
int ezpp(ezpp_t* ez, char* mapfile) {
  int res, r1, r2;
  parser_t parser;
  beatmap_t map;
  diff_calc_t stars;
  pp_params_t params;
  pp_calc_t pp;

  r1 = p_init(&parser);
  r2 = d_init(&stars);
  if (r1 < 0 || r2 < 0) {
    res = al_min(r1, r2);
    goto cleanup;
  }

  if (ez->mode_override) {
    parser.flags = PARSER_OVERRIDE_MODE;
    parser.mode_override = ez->mode_override;
  }

  if (ez->data_size) {
    res = p_map_mem(&parser, &map, mapfile, ez->data_size);
  } else if (!strcmp(mapfile, "-")) {
    res = p_map(&parser, &map, stdin);
  } else {
    FILE* f = fopen(mapfile, "rb");
    if (!f) {
      perror("fopen");
      res = ERR_IO;
    } else {
      res = p_map(&parser, &map, f);
      fclose(f);
    }
  }

  if (res < 0) {
    goto cleanup;
  }

  if (ez->end > 0 && ez->end < map.nobjects) {
    map.nobjects = ez->end;
  }

  if (ez->ar_override) {
    map.ar = ez->ar_override;
  }

  if (ez->od_override) {
    map.od = ez->od_override;
  }

  if (ez->cs_override) {
    map.cs = ez->cs_override;
  }

  res = d_calc(&stars, &map, ez->mods);
  if (res < 0) {
    goto cleanup;
  }

  pp_init(&params);
  params.mods = ez->mods;
  params.combo = ez->combo;
  params.nmiss = ez->nmiss;
  params.score_version = ez->score_version;
  if (ez->accuracy_percent) {
    switch (map.mode) {
      case MODE_STD:
        acc_round(ez->accuracy_percent, map.nobjects, params.nmiss,
          &params.n300, &params.n100, &params.n50);
        break;
      case MODE_TAIKO: {
        int taiko_max_combo = b_max_combo(&map);
        if (taiko_max_combo < 0) {
          res = taiko_max_combo;
          goto cleanup;
        }
        params.max_combo = taiko_max_combo;
        taiko_acc_round(ez->accuracy_percent, taiko_max_combo,
          params.nmiss, &params.n300, &params.n100);
        break;
      }
    }
  } else {
    params.n300 = ez->n300;
    params.n100 = ez->n100;
    params.n50 = ez->n50;
  }
  params.aim = stars.aim;
  params.speed = stars.speed;
  res = b_ppv2p(&map, &pp, &params);
  if (res < 0) {
    goto cleanup;
  }

  ez->stars = stars.total;
  ez->aim_stars = stars.aim;
  ez->speed_stars = stars.speed;
  ez->pp = pp.total;
  ez->aim_pp = pp.aim;
  ez->speed_pp = pp.speed;
  ez->acc_pp = pp.acc;
  ez->accuracy_percent = pp.accuracy * 100.0f;

cleanup:
  p_free(&parser);
  d_free(&stars);
  return res;
}

#endif /* OPPAI_IMPLEMENTATION */

