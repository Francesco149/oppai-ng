/*
 * this is free and unencumbered software released into the
 * public domain.
 *
 * refer to the attached UNLICENSE or http://unlicense.org/
 * ----------------------------------------------------------------
 * command line interface for oppai
 */

#define OPPAI_IMPLEMENTATION
#include "oppai.c"

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))
char* me = "oppai";

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
    "-st[n]\n"
    "  the maximum 1/2 bpm that is considered "
    "singletappable for the singletap stats.\n"
    "  default: 240\n"
    "  example: -st260\n"
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

#define output_sig(name) \
void name(int result, beatmap_t* map, beatmap_stats_t* mapstats, \
  char* mods_str, diff_calc_t* stars, pp_params_t* params, pp_calc_t* pp)

typedef output_sig(fnoutput);

/* null output --------------------------------------------------------- */

/* stdout must be left alone, outputting to stderr is fine tho */
output_sig(output_null) {
  (void)result; (void)map; (void)mapstats; (void)mods_str;
  (void)stars; (void)params; (void)pp;
}

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

  static int charsetsize = ARRAY_LEN(charset);

  float values[ASCIIPLT_W];
  float minval = (float)get_inf();
  float maxval = (float)-get_inf();
  float range;
  int i;
  int chunksize;
  int w = mymin(ASCIIPLT_W, n);

  memset(values, 0, sizeof(values));
  chunksize = (int)ceil((float)n / w);

  for (i = 0; i < n; ++i) {
    int chunki = i / chunksize;
    values[chunki] = mymax(values[chunki], getvalue(data, i));
  }

  for (i = 0; i < n; ++i) {
    int chunki = i / chunksize;
    maxval = mymax(maxval, values[chunki]);
    minval = mymin(minval, values[chunki]);
  }

  range = mymax(0.00001f, maxval - minval);

  for (i = 0; i < w; ++i) {
    int chari = (int)(((values[i] - minval) / range) * charsetsize);
    chari = mymax(0, mymin(chari, charsetsize - 1));
    printf("%s", charset[chari]);
  }

  puts("");
}

float getaim(void* data, int i) {
  beatmap_t* b = data;
  return b->objects[i].strains[DIFF_AIM];
}

float getspeed(void* data, int i) {
  beatmap_t* b = data;
  return b->objects[i].strains[DIFF_SPEED];
}

#define twodec(x) (round_oppai((x) * 100.0f) / 100.0f)

output_sig(output_text) {
  int total_objs;

  if (result < 0) {
    puts(errstr(result));
    return;
  }

  printf("%s - %s ", map->artist, map->title);

  if (strcmp(map->artist, map->artist_unicode) ||
    strcmp(map->title, map->title_unicode))
  {
    printf("(%s - %s) ", map->artist_unicode, map->title_unicode);
  }

  printf("[%s] mapped by %s ", map->version, map->creator);
  puts("\n");

  mapstats->ar = twodec(mapstats->ar);
  mapstats->od = twodec(mapstats->od);
  mapstats->cs = twodec(mapstats->cs);
  mapstats->hp = twodec(mapstats->hp);
  stars->total = twodec(stars->total);
  stars->aim = twodec(stars->aim);
  stars->speed = twodec(stars->speed);

  printf("AR%g OD%g ", mapstats->ar, mapstats->od);

  if (map->mode == MODE_STD) {
    printf("CS%g ", mapstats->cs);
  }

  printf("HP%g\n", mapstats->hp);
  printf("300 hitwindow: %g ms\n", mapstats->odms);

  printf("%d circles, %d sliders, %d spinners\n",
    map->ncircles, map->nsliders, map->nspinners);

  /* -1 because first object can't be evaluated */
  total_objs = map->ncircles + map->nsliders - 1;

  if (map->mode == MODE_STD) {
    printf("%d spacing singletaps (%g%%)\n", stars->nsingles,
      stars->nsingles / (float)total_objs * 100.0);

    printf("%d notes within singletap bpm threshold (%g%%)\n",
      stars->nsingles_threshold,
      stars->nsingles_threshold/(float)total_objs * 100.0);

    puts("");

    printf("%g stars (%g aim, %g speed)\n", stars->total,
      stars->aim, stars->speed);

    printf("\nspeed strain: ");
    asciiplt(getspeed, map->nobjects, map);

    printf("  aim strain: ");
    asciiplt(getaim, map->nobjects, map);
  } else {
    printf("%g stars\n", stars->total);
  }

  printf("\n");

  if (mods_str) {
    printf("+%s ", mods_str);
  }

  printf("%d/%dx ", params->combo, params->max_combo);
  printf("%g%%\n", twodec(pp->accuracy * 100));

  printf("%g pp (", twodec(pp->total));

  if (map->mode == MODE_STD) {
    printf("%g aim, ", twodec(pp->aim));
  }

  printf("%g speed, ", twodec(pp->speed));
  printf("%g acc)\n\n", twodec(pp->acc));
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

int is_inf(float b) {
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
  printf("{\"oppai_version\":\"%s\",", oppai_version_str());

  if (result < 0) {
    printf("\"code\":%d,", result);
    printf("\"errstr\":");
    print_escaped_json_string(errstr(result));
    printf("}");
    return;
  }

  fix_json_flt(&pp->total);
  fix_json_flt(&pp->aim);
  fix_json_flt(&pp->speed);
  fix_json_flt(&pp->accuracy);
  fix_json_flt(&stars->total);
  fix_json_flt(&stars->aim);
  fix_json_flt(&stars->speed);

  printf("\"code\":200,\"errstr\":\"no error\",");

  printf("\"artist\":");
  print_escaped_json_string(map->artist);

  if (strcmp(map->artist, map->artist_unicode)) {
    printf(",\"artist_unicode\":");
    print_escaped_json_string(map->artist_unicode);
  }

  printf(",\"title\":");
  print_escaped_json_string(map->title);

  if (strcmp(map->title, map->title_unicode)) {
    printf(",\"title_unicode\":");
    print_escaped_json_string(map->title_unicode);
  }

  printf(",\"creator\":");
  print_escaped_json_string(map->creator);

  printf(",\"version\":");
  print_escaped_json_string(map->version);

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
    "\"nsingles\":%d,\"nsingles_threshold\":%d,"
    "\"aim_pp\":%.17g,\"speed_pp\":%.17g,\"acc_pp\":%.17g,"
    "\"pp\":%.17g}",
    mods_str, params->mods, mapstats->od, mapstats->ar,
    mapstats->cs, mapstats->hp, params->combo,
    params->max_combo, map->ncircles, map->nsliders,
    map->nspinners, params->nmiss, params->score_version,
    stars->total, stars->speed, stars->aim, stars->nsingles,
    stars->nsingles_threshold, pp->aim, pp->speed, pp->acc,
    pp->total
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
  print_escaped_csv_string(map->artist);
  puts("");

  if (strcmp(map->artist, map->artist_unicode)) {
    printf("artist_unicode;");
    print_escaped_csv_string(map->artist_unicode);
    puts("");
  }

  printf("title;");
  print_escaped_csv_string(map->title);
  puts("");

  if (strcmp(map->title, map->title_unicode)) {
    printf("title_unicode;");
    print_escaped_csv_string(map->title_unicode);
    puts("");
  }

  printf("version;");
  print_escaped_csv_string(map->version);
  puts("");

  printf("creator;");
  print_escaped_csv_string(map->creator);
  puts("");

  if (!mods_str) {
    mods_str = "";
  }

  printf(
    "mods_str;%s\nmods;%d\nod;%g\nar;%g\ncs;%g\nhp;%g\n"
    "combo;%d\nmax_combo;%d\nnum_circles;%d\n"
    "num_sliders;%d\nnum_spinners;%d\nmisses;%d\n"
    "score_version;%d\nstars;%.17g\nspeed_stars;%.17g\n"
    "aim_stars;%.17g\nnsingles;%d\nnsingles_threshold;%d\n"
    "aim_pp;%.17g\nspeed_pp;%.17g\nacc_pp;%.17g\npp;%.17g",
    mods_str, params->mods, mapstats->od, mapstats->ar,
    mapstats->cs, mapstats->hp, params->combo,
    params->max_combo, map->ncircles, map->nsliders,
    map->nspinners, params->nmiss, params->score_version,
    stars->total, stars->speed, stars->aim, stars->nsingles,
    stars->nsingles_threshold, pp->aim, pp->speed, pp->acc,
    pp->total
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
  int len = mymin(0xFFFF, (int)strlen(str));
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
  write_str(map->artist);
  write_str(map->artist_unicode);
  write_str(map->title);
  write_str(map->title_unicode);
  write_str(map->version);
  write_str(map->creator);
  write4(params->mods);
  write_flt(mapstats->od);
  write_flt(mapstats->ar);
  write_flt(mapstats->cs);
  write_flt(mapstats->hp);
  write4(params->combo);
  write4(params->max_combo);
  write2(map->ncircles);
  write2(map->nsliders);
  write2(map->nspinners);
  write4(params->score_version);
  write_flt(stars->total);
  write_flt(stars->speed);
  write_flt(stars->aim);
  write2(stars->nsingles);
  write2(stars->nsingles_threshold);
  write_flt(pp->aim);
  write_flt(pp->speed);
  write_flt(pp->acc);
  write_flt(pp->total);
}

/* gnuplot output ------------------------------------------------------ */

#define gnuplot_string(x) print_escaped_json_string_ex(x, 0)

void gnuplot_strains(beatmap_t* map, int type) {
  int i;
  for (i = 0; i < map->nobjects; ++i) {
    object_t* o = &map->objects[i];
    printf("%.17g %.17g\n", o->time, o->strains[type]);
  }
}

output_sig(output_gnuplot) {
  (void)pp; (void)params; (void)stars; (void)mapstats; (void)result;

  if (map->mode != MODE_STD) {
    return;
  }

  puts("set encoding utf8;");

  printf("set title \"");
  gnuplot_string(map->artist);
  printf(" - ");
  gnuplot_string(map->title);

  if (strcmp(map->artist, map->artist_unicode) ||
    strcmp(map->title, map->title_unicode))
  {
    printf("(");
    gnuplot_string(map->artist_unicode);
    printf(" - ");
    gnuplot_string(map->title_unicode);
    printf(")");
  }

  printf(" [");
  gnuplot_string(map->version);
  printf("] mapped by ");
  gnuplot_string(map->creator);
  if (mods_str) printf(" +%s", mods_str);
  puts("\";");

  puts(
    "set xlabel 'time (ms)';"
    "set ylabel 'strain';"
    "set multiplot layout 2,1 rowsfirst;"
    "plot '-' with lines lc 1 title 'speed'"
  );
  gnuplot_strains(map, DIFF_SPEED);
  puts("e");
  puts("unset title;");
  puts("plot '-' with lines lc 2 title 'aim'");
  gnuplot_strains(map, DIFF_AIM);
}

#ifdef OPPAI_DEBUG
/* debug output -------------------------------------------------------- */

output_sig(output_debug) {
  int i;

  (void)mods_str;
  (void)stars;
  (void)params;

  if (result < 0) {
    puts(errstr(result));
    return;
  }

  for (i = 0; i < map->ntiming_points; ++i) {
    timing_t* t = &map->timing_points[i];
    printf("timing %gms %g %d\n", t->time, t->ms_per_beat, t->change);
  }

  for (i = 0; i < map->nobjects; ++i) {
    object_t* o = &map->objects[i];
    printf("%gs [%g %g] ", o->time / 1000.0, o->strains[0], o->strains[1]);

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

  printf("AR%g OD%g CS%g HP%g\n", mapstats->ar, mapstats->od,
    mapstats->cs, mapstats->hp);

  printf("%d circles, %d sliders, %d spinners\n",
    map->ncircles, map->nsliders, map->nspinners);

  printf("%d/%dx\n", params->combo, params->max_combo);

  printf("%g stars (%g aim, %g speed)\n", stars->total,
    stars->aim, stars->speed);

  printf("%g%%\n", pp->accuracy * 100);
  printf("%g aim pp\n", pp->aim);
  printf("%g speed pp\n", pp->speed);
  printf("%g acc pp\n\n", pp->acc);
  printf("%g pp\n", pp->total);
}
#endif /* OPPAI_DEBUG */

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
#ifdef OPPAI_DEBUG
  { "debug", output_debug, { "debug output", 0 } },
#endif
};

output_module_t* output_by_name(char* name) {
  int i;
  for (i = 0; i < ARRAY_LEN(modules); ++i) {
    if (!strcmp(modules[i].name, name)) {
      return &modules[i];
    }
  }
  return 0;
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

int cmpsuffix(char* str, char* suffix) {
  int sufflen = (int)mymin(strlen(str), strlen(suffix));
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
  FILE* f = 0;
  int i;
  int result;

  parser_t* pstate = 0;
  diff_calc_t stars;
  beatmap_t map;
  beatmap_stats_t mapstats;
  pp_params_t params;
  pp_calc_t pp;
  output_module_t* m;

  char* output_name = "text";
  char* mods_str = 0;
  int mods = MODS_NOMOD;
  float acc_percent = 100.0f;
  int use_percent = 0;

  int overrides = 0;

#define OVERRIDE_AR (1<<0)
#define OVERRIDE_OD (1<<1)
#define OVERRIDE_CS (1<<2)
#define OVERRIDE_SINGLETAP_THRESHOLD (1<<3)
#define OVERRIDE_MODE (1<<4)
#define OVERRIDE_SPEED (1<<5)
#define OVERRIDE_AIM (1<<6)

  float ar_override = 0, od_override = 0, cs_override = 0;
  float singletap_threshold = 125.0f;
  int mode_override = MODE_STD;
  float speed_override = 0, aim_override = 0;
  int end = 0;

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

  pp_init(&params);

  for (i = 2; i < argc; ++i) {
    char* a = argv[i];
    char* p;
    int iswhite = 1;

    for (p = a; *p; ++p) {
      if (!whitespace(*p)) {
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

    if (strlen(a) >= 3 && !memcmp(a, "-st", 3) &&
      sscanf(a + 3, "%f", &singletap_threshold) == 1)
    {
      singletap_threshold = (60000.0f / singletap_threshold) / 2.0f;
      overrides |= OVERRIDE_SINGLETAP_THRESHOLD;
      continue;
    }

    if (!cmpsuffix(a, "%") && sscanf(a, "%f", &acc_percent) == 1) {
      use_percent = 1;
      continue;
    }

    if (!cmpsuffix(a, "x100") && sscanf(a, "%d", &params.n100) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "x50") && sscanf(a, "%d", &params.n50) == 1) {
      continue;
    }

    if (!cmpsuffix(a, "speed") && sscanf(a, "%f", &speed_override) == 1) {
      overrides |= OVERRIDE_SPEED;
      continue;
    }

    if (!cmpsuffix(a, "aim") && sscanf(a, "%f", &aim_override) == 1) {
      overrides |= OVERRIDE_AIM;
      continue;
    }

    if (!cmpsuffix(a, "xm") || !cmpsuffix(a, "xmiss") ||
      !cmpsuffix(a, "m"))
    {
      if (sscanf(a, "%d", &params.nmiss) == 1) {
        continue;
      }
    }

    if (!cmpsuffix(a, "x") && sscanf(a, "%d", &params.combo) == 1) {
      continue;
    }

    if (sscanf(a, "scorev%d", &params.score_version)) {
      continue;
    }

    if (sscanf(a, "ar%f", &ar_override)) {
      overrides |= OVERRIDE_AR;
      continue;
    }

    if (sscanf(a, "od%f", &od_override)) {
      overrides |= OVERRIDE_OD;
      continue;
    }

    if (sscanf(a, "cs%f", &cs_override)) {
      overrides |= OVERRIDE_CS;
      continue;
    }

    if (sscanf(a, "-m%d", &mode_override) == 1) {
      overrides |= OVERRIDE_MODE;
      continue;
    }

    if (sscanf(a, "-end%d", &end) == 1) {
      continue;
    }

    if (!strcmp(a, "-taiko")) {
      overrides |= OVERRIDE_MODE;
      mode_override = MODE_TAIKO;
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

  /* parse beatmap --------------------------------------------------- */
  if (!strcmp(argv[1], "-")) {
    f = stdin;
  } else {
    f = fopen(argv[1], "rb");
    if (!f) {
      perror("fopen");
      result = ERR_IO;
      goto output;
    }
  }

  pstate = malloc(sizeof(parser_t));
  if (!pstate) {
    result = ERR_OOM;
    goto output;
  }

  result = p_init(pstate);
  if (result < 0) {
    goto output;
  }

  if (overrides & OVERRIDE_MODE) {
    pstate->mode_override = mode_override;
    pstate->flags = PARSER_OVERRIDE_MODE;
  }

  result = p_map(pstate, &map, f);
  if (result < 0) {
    info("last parser line: ");
    slice_write(&pstate->lastline, stderr);
    info("\n");
    info("last parser position: ");
    slice_write(&pstate->lastpos, stderr);
    info("\n");
    goto output;
  }

  if (overrides & OVERRIDE_AR) {
    map.ar = ar_override;
  }

  if (overrides & OVERRIDE_OD) {
    map.od = od_override;
  }

  if (overrides & OVERRIDE_CS) {
    map.cs = cs_override;
  }

  if (end > 0 && end < map.nobjects) {
    map.nobjects = end;
  }

  /* diff calc --------------------------------------------------------- */
  result = d_init(&stars);
  if (result < 0) {
    goto output;
  }

  if (overrides & OVERRIDE_SINGLETAP_THRESHOLD) {
    stars.singletap_threshold = singletap_threshold;
  }

  result = d_calc(&stars, &map, mods);
  if (result < 0) {
    goto output;
  }

  if (overrides & OVERRIDE_AIM) {
    stars.aim = aim_override;
  }

  if (overrides & OVERRIDE_SPEED) {
    stars.speed = speed_override;
    if (map.mode == MODE_TAIKO) {
      stars.total = stars.speed;
    }
  }

  /* pp calc ------------------------------------------------- */
  mapstats.ar = map.ar;
  mapstats.cs = map.cs;
  mapstats.od = map.od;
  mapstats.hp = map.hp;

  mods_apply_m(map.mode, mods, &mapstats, APPLY_ALL);

  params.aim = stars.aim;
  params.speed = stars.speed;
  params.mods = mods;

  if (use_percent) {
    switch (map.mode) {
      case MODE_STD:
        acc_round(acc_percent, map.nobjects, params.nmiss, &params.n300,
          &params.n100, &params.n50);
        break;
      case MODE_TAIKO: {
        int taiko_max_combo = b_max_combo(&map);
        if (taiko_max_combo < 0) {
          result = taiko_max_combo;
          goto output;
        }
        params.max_combo = (int)taiko_max_combo;
        taiko_acc_round(acc_percent, (int)taiko_max_combo,
          params.nmiss, &params.n300, &params.n100);
        break;
      }
    }
  }

  result = b_ppv2p(&map, &pp, &params);

  /* output ---------------------------------------------------------- */
output:
  print_memory_usage(pstate, &stars);

  m = output_by_name(output_name);
  if (!m) {
    info("output module '%s' does not exist. check 'oppai - -o?'\n",
      output_name);
    return 1;
  }

  m->func(result, &map, &mapstats, mods_str, &stars, &params, &pp);

  /* this cleanup is only here so that valgrind stops crying */
  p_free(pstate);
  d_free(&stars);
  free(pstate);
  if (f && f != stdin) {
    fclose(f);
  }

  return result < 0;
}

