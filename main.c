/*
 * this is free and unencumbered software released into the
 * public domain.
 *
 * refer to the attached UNLICENSE or http://unlicense.org/
 * ----------------------------------------------------------------
 * command line interface for oppai
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#undef OPPAI_EXPORT
#undef OPPAI_IMPLEMENTATION
#include "oppai.c"

char* me = "oppai";

#define al_round(x) (float)floor((x) + 0.5f)
#define al_min(a, b) ((a) < (b) ? (a) : (b))
#define al_max(a, b) ((a) > (b) ? (a) : (b))
#define twodec(x) (al_round((x) * 100.0f) / 100.0f)
#define array_len(x) (sizeof(x) / sizeof((x)[0]))

static
float get_inf() {
  static unsigned raw = 0x7F800000;
  float* p = (float*)&raw;
  return *p;
}

static
int is_nan(float b) {
  int* p = (int*)&b;
  return (
    (*p > 0x7F800000 && *p < 0x80000000) ||
    (*p > 0x7FBFFFFF && *p <= 0xFFFFFFFF)
  );
}

static
int info(char* fmt, ...) {
  int res;
  va_list va;
  va_start(va, fmt);
  res = vfprintf(stderr, fmt, va);
  va_end(va);
  return res;
}

void usage() {
  /* logo by flesnuk https://github.com/Francesco149/oppai-ng/issues/10 */

  info(
    "     /\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb"
    "\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb/ /\xe2\x8e\xbb\xe2"
    "\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb"
    "\xe2\x8e\xbb/ /\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2"
    "\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb/ /\xe2\x8e"
    "\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2"
    "\x8e\xbb\xe2\x8e\xbb/ /\xe2\x8e\xbb/      /\xe2\x8e\xbb"
    "\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\\    /\xe2\x8e\xbb"
    "\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e"
    "\xbb\xe2\x8e\xbb/\n    / /\xe2\x8e\xbb\xe2\x8e\xbb\xe2"
    "\x8e\xbb/ / / /\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb/ / "
  );

  info(
    "/ /\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb/ /  \xe2\x8e\xbb"
    "\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb/ / / / "
    "___  / /\xe2\x8e\xbb\xe2\x8e\xbb\\ \\  / /\xe2\x8e\xbb"
    "\xe2\x8e\xbb\xe2\x8e\xbb/ /\n   / /   / / / /   / / / /"
    "   / / /\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb\xe2\x8e\xbb"
    "\xe2\x8e\xbb/ / / / /__/ / /   / / / /   / /\n  / /___/ "
    "/ / /___/ / / /___/ / / /___/ / / /      / /   / / / /__"
    "_/ /\n /_______/ / ______/ / ______/ /_______/ /_/      "
    "/_/   /_/ /_____  /\n          / /       / /            "
    "                              / / \n         / /       /"
    " /                                    /\xe2\x8e\xbb/___/"
    " / \n        /_/       /_/                              "
    "      /_______/"
  );

  info("\n\n");
  info("usage: %s /path/to/file.osu parameters\n\n", me);

  info(
    "set filename to '-' to read from standard input\n"
    "all parameters are case insensitive\n"
    "\n"
    "-o[output_module]\n"
    "  output module. pass ? to list modules (oppai - -o?)\n"
    "  default: text\n"
    "  example: -ojson\n"
    "\n"
    "[accuracy]%%\n"
    "  accuracy percentage\n"
    "  default: 100%%\n"
    "  example: 95%%\n"
    "\n"
    "[n]x100\n"
    "  amount of 100s\n"
    "  default: 0\n"
    "  example: 2x100\n"
    "\n"
  );

  info(
    "[n]x50\n"
    "  amount of 50s\n"
    "  default: 0\n"
    "  example: 2x50\n"
    "\n"
    "[n]xm\n"
    "[n]xmiss\n"
    "[n]m\n"
    "  amount of misses\n"
    "  default: 0\n"
    "  example: 1m\n"
    "\n"
    "[combo]x\n"
    "  highest combo achieved\n"
    "  default: full combo (calculated from map data)\n"
    "  example: 500x\n"
    "\n"
    "scorev[n]\n"
    "  scoring system\n"
    "  default: 1\n"
    "  example: scorev2\n"
    "\n"
  );

  info(
    "ar[n]\n"
    "  base approach rate override\n"
    "  default: map's base approach rate\n"
    "  example: AR5\n"
    "\n"
    "od[n]\n"
    "  base overall difficulty override\n"
    "  default: map's base overall difficulty\n"
    "  example: OD10\n"
    "\n"
    "cs[n]\n"
    "  base circle size override\n"
    "  default: map's base circle size\n"
    "  example: CS6.5\n"
    "\n"
  );

  info(
    "-m[n]\n"
    "  gamemode id override for converted maps\n"
    "  default: uses the map's gamemode\n"
    "  example: -m1\n"
    "\n"
    "-taiko\n"
    "  forces gamemode to taiko for converted maps\n"
    "  default: disabled\n"
    "\n"
    "-touch\n"
    "  calculates pp for touchscreen / touch devices. can \n"
    "  also be specified as mod TD\n"
    "\n"
    "[n]speed\n"
    "  override speed stars. "
    "useful for maps with incorrect star rating\n"
    "  default: uses computed speed stars\n"
    "  example: 3.5speed\n"
    "\n"
  );

  info(
    "[n]aim\n"
    "  override aim stars. "
    "useful for maps with incorrect star rating\n"
    "  default: uses computed aim stars\n"
    "  example: 2.4aim\n"
    "\n"
    "-end[n]\n"
    "  cuts map to a certain number of objects\n"
  );
}

#define output_sig(name) void name(int result, ezpp_t ez, char* mods_str)

typedef output_sig(fnoutput);

/* null output --------------------------------------------------------- */

/* stdout must be left alone, outputting to stderr is fine tho */
output_sig(output_null) { (void)result; (void)ez; (void)mods_str; }

/* text output --------------------------------------------------------- */

#define ASCIIPLT_W 51

void asciiplt(float (* getvalue)(void* data, int i), int n, void* data) {
  static char* charset[] = {
#ifdef OPPAI_UTF8GRAPH
    "\xe2\x96\x81",
    "\xe2\x96\x82",
    "\xe2\x96\x83",
    "\xe2\x96\x84",
    "\xe2\x96\x85",
    "\xe2\x96\x86",
    "\xe2\x96\x87",
    "\xe2\x96\x88"
#else
    " ", "_", ".", "-", "^"
#endif
  };

  static int charsetsize = array_len(charset);

  float values[ASCIIPLT_W];
  float minval = (float)get_inf();
  float maxval = (float)-get_inf();
  float range;
  int i;
  int chunksize;
  int w = al_min(ASCIIPLT_W, n);

  memset(values, 0, sizeof(values));
  chunksize = (int)ceil((float)n / w);

  for (i = 0; i < n; ++i) {
    int chunki = i / chunksize;
    values[chunki] = al_max(values[chunki], getvalue(data, i));
  }

  for (i = 0; i < n; ++i) {
    int chunki = i / chunksize;
    maxval = al_max(maxval, values[chunki]);
    minval = al_min(minval, values[chunki]);
  }

  range = al_max(0.00001f, maxval - minval);

  for (i = 0; i < w; ++i) {
    int chari = (int)(((values[i] - minval) / range) * charsetsize);
    chari = al_max(0, al_min(chari, charsetsize - 1));
    printf("%s", charset[chari]);
  }

  puts("");
}

float getaim(void* data, int i) {
  ezpp_t ez = data;
  return ezpp_strain_at(ez, i, DIFF_AIM);
}

float getspeed(void* data, int i) {
  ezpp_t ez = data;
  return ezpp_strain_at(ez, i, DIFF_SPEED);
}

output_sig(output_text) {
  float ar, od, cs, hp, stars, aim_stars, speed_stars, accuracy_percent;
  float pp, aim_pp, speed_pp, acc_pp;

  if (result < 0) {
    puts(errstr(result));
    return;
  }

  printf("%s - %s ", ezpp_artist(ez), ezpp_title(ez));

  if (strcmp(ezpp_artist(ez), ezpp_artist_unicode(ez)) ||
    strcmp(ezpp_title(ez), ezpp_title_unicode(ez)))
  {
    printf("(%s - %s) ", ezpp_artist_unicode(ez), ezpp_title_unicode(ez));
  }

  printf("[%s] mapped by %s ", ezpp_version(ez), ezpp_creator(ez));
  puts("\n");

  ar = twodec(ezpp_ar(ez));
  od = twodec(ezpp_od(ez));
  cs = twodec(ezpp_cs(ez));
  hp = twodec(ezpp_hp(ez));
  stars = twodec(ezpp_stars(ez));
  aim_stars = twodec(ezpp_aim_stars(ez));
  speed_stars = twodec(ezpp_speed_stars(ez));
  accuracy_percent = twodec(ezpp_accuracy_percent(ez));
  pp = twodec(ezpp_pp(ez));
  aim_pp = twodec(ezpp_aim_pp(ez));
  speed_pp = twodec(ezpp_speed_pp(ez));
  acc_pp = twodec(ezpp_acc_pp(ez));

  printf("AR%g OD%g ", ar, od);

  if (ezpp_mode(ez) == MODE_STD) {
    printf("CS%g ", cs);
  }

  printf("HP%g\n", hp);
  printf("300 hitwindow: %g ms\n", ezpp_odms(ez));

  printf("%d circles, %d sliders, %d spinners\n",
    ezpp_ncircles(ez), ezpp_nsliders(ez), ezpp_nspinners(ez));

  if (ezpp_mode(ez) == MODE_STD) {
    printf("%g stars (%g aim, %g speed)\n", stars, aim_stars, speed_stars);
    printf("\nspeed strain: ");
    asciiplt(getspeed, ezpp_nobjects(ez), ez);
    printf("  aim strain: ");
    asciiplt(getaim, ezpp_nobjects(ez), ez);
  } else {
    printf("%g stars\n", ezpp_stars(ez));
  }

  printf("\n");

  if (mods_str) {
    printf("+%s ", mods_str);
  }

  printf("%d/%dx ", ezpp_combo(ez), ezpp_max_combo(ez));
  printf("%g%%\n", accuracy_percent);
  printf("%g pp (", pp);

  if (ezpp_mode(ez) == MODE_STD) {
    printf("%g aim, ", aim_pp);
  }

  printf("%g speed, ", speed_pp);
  printf("%g acc)\n\n", acc_pp);
}

/* json output --------------------------------------------------------- */

void print_escaped_json_string_ex(char* str, int quotes) {
  char* chars_to_escape = "\\\"";
  char* p;
  if (quotes) {
    putchar('"');
  }
  for (; *str; ++str) {
    /* escape all characters in chars_to_escape */
    for (p = chars_to_escape; *p; ++p) {
      if (*p == *str) {
        putchar('\\');
      }
    }
    putchar(*str);
  }
  if (quotes) {
    putchar('"');
  }
}

#define print_escaped_json_string(x) \
  print_escaped_json_string_ex(x, 1)

/* https://www.doc.ic.ac.uk/%7Eeedwards/compsys/float/nan.html */

static int is_inf(float b) {
  int* p = (int*)&b;
  return *p == 0x7F800000 || *p == 0xFF800000;
}

/*
 * json is mentally challenged and can't handle inf and nan so
 * we're gonna be mathematically incorrect
 */
void fix_json_flt(float* v) {
  if (is_inf(*v)) {
    *v = -1;
  } else if (is_nan(*v)) {
    *v = 0;
  }
}

output_sig(output_json) {
  float pp, aim_pp, speed_pp, acc_pp, stars, aim_stars, speed_stars;
  printf("{\"oppai_version\":\"%s\",", oppai_version_str());

  if (result < 0) {
    printf("\"code\":%d,", result);
    printf("\"errstr\":");
    print_escaped_json_string(errstr(result));
    printf("}");
    return;
  }

  pp = ezpp_pp(ez);
  aim_pp = ezpp_aim_pp(ez);
  speed_pp = ezpp_speed_pp(ez);
  acc_pp = ezpp_acc_pp(ez);
  stars = ezpp_stars(ez);
  aim_stars = ezpp_aim_stars(ez);
  speed_stars = ezpp_speed_stars(ez);
  fix_json_flt(&pp);
  fix_json_flt(&aim_pp);
  fix_json_flt(&speed_pp);
  fix_json_flt(&acc_pp);
  fix_json_flt(&stars);
  fix_json_flt(&aim_stars);
  fix_json_flt(&speed_stars);

  printf("\"code\":200,\"errstr\":\"no error\",");

  printf("\"artist\":");
  print_escaped_json_string(ezpp_artist(ez));

  if (strcmp(ezpp_artist(ez), ezpp_artist_unicode(ez))) {
    printf(",\"artist_unicode\":");
    print_escaped_json_string(ezpp_artist_unicode(ez));
  }

  printf(",\"title\":");
  print_escaped_json_string(ezpp_title(ez));

  if (strcmp(ezpp_title(ez), ezpp_title_unicode(ez))) {
    printf(",\"title_unicode\":");
    print_escaped_json_string(ezpp_title_unicode(ez));
  }

  printf(",\"creator\":");
  print_escaped_json_string(ezpp_creator(ez));

  printf(",\"version\":");
  print_escaped_json_string(ezpp_version(ez));

  printf(",");

  if (!mods_str) {
    mods_str = "";
  }

  printf(
    "\"mods_str\":\"%s\",\"mods\":%d,"
    "\"od\":%g,\"ar\":%g,\"cs\":%g,\"hp\":%g,"
    "\"combo\":%d,\"max_combo\":%d,"
    "\"num_circles\":%d,\"num_sliders\":%d,"
    "\"num_spinners\":%d,\"misses\":%d,"
    "\"score_version\":%d,\"stars\":%.17g,"
    "\"speed_stars\":%.17g,\"aim_stars\":%.17g,"
    "\"aim_pp\":%.17g,\"speed_pp\":%.17g,\"acc_pp\":%.17g,"
    "\"pp\":%.17g}",
    mods_str, ezpp_mods(ez), ezpp_od(ez), ezpp_ar(ez),
    ezpp_cs(ez), ezpp_hp(ez), ezpp_combo(ez),
    ezpp_max_combo(ez), ezpp_ncircles(ez), ezpp_nsliders(ez),
    ezpp_nspinners(ez), ezpp_nmiss(ez), ezpp_score_version(ez),
    ezpp_stars(ez), ezpp_speed_stars(ez), ezpp_aim_stars(ez),
    ezpp_aim_pp(ez), ezpp_speed_pp(ez), ezpp_acc_pp(ez), ezpp_pp(ez)
  );
}

/* csv output ---------------------------------------------------------- */

void print_escaped_csv_string(char* str) {
  char* chars_to_escape = "\\;";
  char* p;
  for (; *str; ++str) {
    /* escape all characters in chars_to_escape */
    for (p = chars_to_escape; *p; ++p) {
      if (*p == *str) {
        putchar('\\');
      }
    }
    putchar(*str);
  }
}

output_sig(output_csv) {
  printf("oppai_version;%s\n", oppai_version_str());

  if (result < 0) {
    printf("code;%d\nerrstr;", result);
    print_escaped_csv_string(errstr(result));
    return;
  }

  printf("code;200\nerrstr;no error\n");

  printf("artist;");
  print_escaped_csv_string(ezpp_artist(ez));
  puts("");

  if (strcmp(ezpp_artist(ez), ezpp_artist_unicode(ez))) {
    printf("artist_unicode;");
    print_escaped_csv_string(ezpp_artist_unicode(ez));
    puts("");
  }

  printf("title;");
  print_escaped_csv_string(ezpp_title(ez));
  puts("");

  if (strcmp(ezpp_title(ez), ezpp_title_unicode(ez))) {
    printf("title_unicode;");
    print_escaped_csv_string(ezpp_title_unicode(ez));
    puts("");
  }

  printf("version;");
  print_escaped_csv_string(ezpp_version(ez));
  puts("");

  printf("creator;");
  print_escaped_csv_string(ezpp_creator(ez));
  puts("");

  if (!mods_str) {
    mods_str = "";
  }

  printf(
    "mods_str;%s\nmods;%d\nod;%g\nar;%g\ncs;%g\nhp;%g\n"
    "combo;%d\nmax_combo;%d\nnum_circles;%d\n"
    "num_sliders;%d\nnum_spinners;%d\nmisses;%d\n"
    "score_version;%d\nstars;%.17g\nspeed_stars;%.17g\n"
    "aim_stars;%.17g\naim_pp;%.17g\nspeed_pp;%.17g\nacc_pp;%.17g\npp;%.17g",
    mods_str, ezpp_mods(ez), ezpp_od(ez), ezpp_ar(ez),
    ezpp_cs(ez), ezpp_hp(ez), ezpp_combo(ez),
    ezpp_max_combo(ez), ezpp_ncircles(ez), ezpp_nsliders(ez),
    ezpp_nspinners(ez), ezpp_nmiss(ez), ezpp_score_version(ez),
    ezpp_stars(ez), ezpp_speed_stars(ez), ezpp_aim_stars(ez),
    ezpp_aim_pp(ez), ezpp_speed_pp(ez), ezpp_acc_pp(ez), ezpp_pp(ez)
  );
}

/* binary output ------------------------------------------------------- */

void write1(int v) {
  char buf = (char)(v & 0xFF);
  fwrite(&buf, 1, 1, stdout);
}

void write2(int v) {
  char buf[2];
  buf[0] = (char)(v & 0xFF);
  buf[1] = (char)(v >> 8);
  fwrite(buf, 1, 2, stdout);
}

void write4(int v) {
  char buf[4];
  buf[0] = (char)(v & 0xFF);
  buf[1] = (char)((v >> 8) & 0xFF);
  buf[2] = (char)((v >> 16) & 0xFF);
  buf[3] = (char)((v >> 24) & 0xFF);
  fwrite(buf, 1, 4, stdout);
}

void write_flt(float f) {
  int* p = (int*)&f;
  write4(*p);
}

void write_str(char* str) {
  int len = al_min(0xFFFF, (int)strlen(str));
  write2(len);
  printf("%s", str);
  write1(0);
}

output_sig(output_binary) {
  int major, minor, patch;
  (void)mods_str;

  if (!freopen(0, "wb", stdout)) {
    perror("freopen");
    exit(1);
  }

  printf("binoppai");
  oppai_version(&major, &minor, &patch);
  write1(major);
  write1(minor);
  write1(patch);
  write4(result);

  if (result < 0) {
    return;
  }

  /* TODO: use varargs to group calls of the same func */
  write_str(ezpp_artist(ez));
  write_str(ezpp_artist_unicode(ez));
  write_str(ezpp_title(ez));
  write_str(ezpp_title_unicode(ez));
  write_str(ezpp_version(ez));
  write_str(ezpp_creator(ez));
  write4(ezpp_mods(ez));
  write_flt(ezpp_od(ez));
  write_flt(ezpp_ar(ez));
  write_flt(ezpp_cs(ez));
  write_flt(ezpp_hp(ez));
  write4(ezpp_combo(ez));
  write4(ezpp_max_combo(ez));
  write2(ezpp_ncircles(ez));
  write2(ezpp_nsliders(ez));
  write2(ezpp_nspinners(ez));
  write4(ezpp_score_version(ez));
  write_flt(ezpp_stars(ez));
  write_flt(ezpp_speed_stars(ez));
  write_flt(ezpp_aim_stars(ez));
  write2(0); /* legacy (nsingles) */
  write2(0); /* legacy (nsigles_threshold) */
  write_flt(ezpp_aim_pp(ez));
  write_flt(ezpp_speed_pp(ez));
  write_flt(ezpp_acc_pp(ez));
  write_flt(ezpp_pp(ez));
}

/* gnuplot output ------------------------------------------------------ */

#define gnuplot_string(x) print_escaped_json_string_ex(x, 0)

void gnuplot_strains(ezpp_t ez, int type) {
  int i;
  for (i = 0; i < ezpp_nobjects(ez); ++i) {
    printf("%.17g %.17g\n", ezpp_time_at(ez, i),
      ezpp_strain_at(ez, i, type));
  }
}

output_sig(output_gnuplot) {
  if (result < 0 || ezpp_mode(ez) != MODE_STD) {
    return;
  }

  puts("set encoding utf8;");

  printf("set title \"");
  gnuplot_string(ezpp_artist(ez));
  printf(" - ");
  gnuplot_string(ezpp_title(ez));

  if (strcmp(ezpp_artist(ez), ezpp_artist_unicode(ez)) ||
    strcmp(ezpp_title(ez), ezpp_title_unicode(ez)))
  {
    printf("(");
    gnuplot_string(ezpp_artist_unicode(ez));
    printf(" - ");
    gnuplot_string(ezpp_title_unicode(ez));
    printf(")");
  }

  printf(" [");
  gnuplot_string(ezpp_version(ez));
  printf("] mapped by ");
  gnuplot_string(ezpp_creator(ez));
  if (mods_str) printf(" +%s", mods_str);
  puts("\";");

  puts(
    "set xlabel 'time (ms)';"
    "set ylabel 'strain';"
    "set multiplot layout 2,1 rowsfirst;"
    "plot '-' with lines lc 1 title 'speed'"
  );
  gnuplot_strains(ez, DIFF_SPEED);
  puts("e");
  puts("unset title;");
  puts("plot '-' with lines lc 2 title 'aim'");
  gnuplot_strains(ez, DIFF_AIM);
}

/* ------------------------------------------------------------- */

#define CODE_DESC "the code and errstr fields " \
  "should be checked for errors. a negative value for code " \
  "indicates an error"

typedef struct output_module {
  char* name;
  fnoutput* func;
  char* description[4];
  /* null terminated array of strings because of c90 literal limits */
} output_module_t;

output_module_t modules[] = {
  { "null", output_null, { "no output", 0 } },
  { "text", output_text, { "plain text", 0 } },
  {
    "json", output_json,
    { "a single utf-8 json object.\n" CODE_DESC, 0 }
  },
  {
    "csv", output_csv,
    { "fieldname;value\n"
    "one value per line. ';' characters in strings will be " 
    "escaped to \"\\;\". utf-8.\n" CODE_DESC, 0 }
  },
  {
    "binary",
    output_binary,
    { "binary stream of values, encoded in little endian.\n"
    "negative code values indicate an error, which matches "
    "the error codes defined in oppai.c\n"
    "for an example on how to read this in C, check out "
    "examples/binary.c in oppai-ng's source\n"
    "\n"
    "floats and floats are represented using whatever "
    "convention the host machine and compiler use. unless you "
    "are on a really exotic machine it shouldn't matter\n"
    "\n"
    "strings (str) are encoded as a 2-byte integer indicating "
    "the length in bytes, followed by the string bytes and ",
    "a null (zero) terminating byte\n"
    "\n"
    "binoppai (8-byte magic), "
    "int8 oppai_ver_major, int8 oppai_ver_minor, "
    "int8 oppai_ver_patch, int error_code, "
    "str artist, str artist_utf8, str title, str title_utf8, "
    "str version, str creator, "
    "int mods_bitmask, float od, float ar, float cs, "
    "float hp, int combo, int max_combo, "
    "int16 ncircles, int16 nsliders, int16 nspinner, "
    "int score_version, float total_stars, ",
    "float speed_stars, float aim_stars, int16 nsingles, "
    "int16 nsingles_threshold, float aim_pp, "
    "float speed_pp, float acc_pp, float pp",
    0 }
  },
  { "gnuplot", output_gnuplot, { "gnuplot .gp script", 0 } },
};

output_module_t* output_by_name(char* name) {
  int i;
  for (i = 0; i < array_len(modules); ++i) {
    if (!strcmp(modules[i].name, name)) {
      return &modules[i];
    }
  }
  return 0;
}

int cmpsuffix(char* str, char* suffix) {
  int sufflen = (int)al_min(strlen(str), strlen(suffix));
  return strcmp(str + strlen(str) - sufflen, suffix);
}

char lowercase(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  }
  return c;
}

char uppercase(char c) {
  if (c >= 'a' && c <= 'z') {
    return c - ('a' - 'A');
  }
  return c;
}

int strcmp_nc(char* a, char* b) {
  for (;; ++a, ++b) {
    char la = lowercase(*a);
    char lb = lowercase(*b);
    if (la > lb) {
      return 1;
    }
    else if (la < lb) {
      return -1;
    }
    if (!*a || *b) {
      break;
    }
  }
  return 0;
}

/* TODO: split main into smaller funcs for readability? */
int main(int argc, char* argv[]) {
  int i;
  int result;
  ezpp_t ez = ezpp_new();
  output_module_t* m;
  char* output_name = "text";
  char* mods_str = 0;
  int mods = MODS_NOMOD;
  float tmpf, speed_stars = 0, aim_stars = 0, accuracy_percent = 0;
  int tmpi, n100 = 0, n50 = 0;

  /* parse arguments ------------------------------------------------- */
  me = argv[0];

  if (argc < 2) {
    usage();
    return 1;
  }

  if (*argv[1] == '-' && strlen(argv[1]) > 1) {
    char* a = argv[1] + 1;
    if (!strcmp_nc(a, "version") || !strcmp_nc(a, "v")) {
      puts(oppai_version_str());
      return 0;
    }
  }

  for (i = 2; i < argc; ++i) {
    char* a = argv[i];
    char* p;
    int iswhite = 1;

    for (p = a; *p; ++p) {
      if (!isspace(*p)) {
        iswhite = 0;
        break;
      }
    }

    if (iswhite) {
      continue;
    }

    for (p = a; *p; ++p) {
      *p = lowercase(*p);
    }

    if (*a == '-' && a[1] == 'o') {
      output_name = a + 2;

      if (!strcmp(output_name, "?")) {
        int j;
        int nmodules = sizeof(modules) / sizeof(modules[0]);
        for (j = 0; j < nmodules; ++j) {
          char** d = modules[j].description;
          puts(modules[j].name);
          for (; *d; ++d) {
            printf("%s", *d);
          }
          puts("\n-");
        }
        return 0;
      }

      continue;
    }

    if (!cmpsuffix(a, "%") && sscanf(a, "%f", &accuracy_percent) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "x100") && sscanf(a, "%d", &n100) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "x50") && sscanf(a, "%d", &n50) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "speed") && sscanf(a, "%f", &speed_stars) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "aim") && sscanf(a, "%f", &aim_stars) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "xm") || !cmpsuffix(a, "xmiss") ||
      !cmpsuffix(a, "m"))
    {
      if (sscanf(a, "%d", &tmpi) == 1) {
        ezpp_set_nmiss(ez, tmpi);
        continue;
      }
    }

    if (!cmpsuffix(a, "x") && sscanf(a, "%d", &tmpi) == 1) {
      ezpp_set_combo(ez, tmpi);
      continue;
    }

    if (sscanf(a, "scorev%d", &tmpi)) {
      ezpp_set_score_version(ez, tmpi);
      continue;
    }

    if (sscanf(a, "ar%f", &tmpf)) {
      ezpp_set_base_ar(ez, tmpf);
      continue;
    }

    if (sscanf(a, "od%f", &tmpf)) {
      ezpp_set_base_od(ez, tmpf);
      continue;
    }

    if (sscanf(a, "cs%f", &tmpf)) {
      ezpp_set_base_cs(ez, tmpf);
      continue;
    }

    if (sscanf(a, "-m%d", &tmpi) == 1) {
      ezpp_set_mode_override(ez, tmpi);
      continue;
    }

    if (sscanf(a, "-end%d", &tmpi) == 1) {
      ezpp_set_end(ez, tmpi);
      continue;
    }

    if (!strcmp(a, "-taiko")) {
      ezpp_set_mode_override(ez, MODE_TAIKO);
      continue;
    }

    if (!strcmp(a, "-touch")) {
      mods |= MODS_TOUCH_DEVICE;
      continue;
    }

    /* this should be last because it uppercase's the string */
    if (*a == '+') {
      mods_str = a + 1;
      for (p = mods_str; *p; ++p) {
        *p = uppercase(*p);
      }

      #define m(mod) \
        if (!strncmp(p, #mod, strlen(#mod))) { \
          mods |= MODS_##mod; \
          p += strlen(#mod); \
          continue; \
        }

      for (p = mods_str; *p;) {
        m(NF) m(EZ) m(TD) m(HD) m(HR) m(SD) m(DT) m(RX) m(HT) m(NC) m(FL)
        m(AT) m(SO) m(AP) m(PF) m(NOMOD)
        ++p;
      }

      #undef m
      continue;
    }

    info(">%s\n", a);
    result = ERR_SYNTAX;
    goto output;
  }

  if (accuracy_percent) {
    ezpp_set_accuracy_percent(ez, accuracy_percent);
  } else {
    ezpp_set_accuracy(ez, n100, n50);
  }
  ezpp_set_mods(ez, mods);
  ezpp_set_speed_stars(ez, speed_stars);
  ezpp_set_aim_stars(ez, aim_stars);
  result = ezpp(ez, argv[1]);

output:
  m = output_by_name(output_name);
  if (!m) {
    info("output module '%s' does not exist. check 'oppai - -o?'\n",
      output_name);
    return 1;
  }

  m->func(result, ez, mods_str);
  ezpp_free(ez); /* just so valgrind stops crying */
  return result < 0;
}

