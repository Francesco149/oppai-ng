/* this is free and unencumbered software released into the
   public domain.

   refer to the attached UNLICENSE or http://unlicense.org/
   ----------------------------------------------------------------
   command line interface for oppai                              */

#define OPPAI_IMPLEMENTATION
#include "oppai.c"

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define OPPAI_VERSION_STRING \
    STRINGIFY(OPPAI_VERSION_MAJOR) "." \
    STRINGIFY(OPPAI_VERSION_MINOR) "." \
    STRINGIFY(OPPAI_VERSION_PATCH)

global char const* me = "oppai";


internalfn
void usage()
{
    /* logo by flesnuk
    https://github.com/Francesco149/oppai-ng/issues/10 */

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
        "    output module. pass ? to list modules (oppai - -o?)\n"
        "    default: text\n"
        "    example: -ojson\n"
        "\n"
        "[accuracy]%%\n"
        "    accuracy percentage\n"
        "    default: 100%%\n"
        "    example: 95%%\n"
        "\n"
        "[n]x100\n"
        "    amount of 100s\n"
        "    default: 0\n"
        "    example: 2x100\n"
        "\n"
    );

    info(
        "[n]x50\n"
        "    amount of 50s\n"
        "    default: 0\n"
        "    example: 2x50\n"
        "\n"
        "[n]xm\n"
        "[n]xmiss\n"
        "[n]m\n"
        "    amount of misses\n"
        "    default: 0\n"
        "    example: 1m\n"
        "\n"
        "[combo]x\n"
        "    highest combo achieved\n"
        "    default: full combo (calculated from map data)\n"
        "    example: 500x\n"
        "\n"
        "scorev[n]\n"
        "    scoring system\n"
        "    default: 1\n"
        "    example: scorev2\n"
        "\n"
    );

    info(
        "ar[n]\n"
        "    base approach rate override\n"
        "    default: map's base approach rate\n"
        "    example: AR5\n"
        "\n"
        "od[n]\n"
        "    base overall difficulty override\n"
        "    default: map's base overall difficulty\n"
        "    example: OD10\n"
        "\n"
        "cs[n]\n"
        "    base circle size override\n"
        "    default: map's base circle size\n"
        "    example: CS6.5\n"
        "\n"
        "-st[n]\n"
        "    the maximum 1/2 bpm that is considered "
        "singletappable for the singletap stats.\n"
        "    default: 240\n"
        "    example: -st260\n"
        "\n"
    );

    info(
        "-m[n]\n"
        "    gamemode id override for converted maps\n"
        "    default: uses the map's gamemode\n"
        "    example: -m1\n"
        "\n"
        "-taiko\n"
        "    forces gamemode to taiko for converted maps\n"
        "    default: disabled\n"
        "\n"
        "-touch\n"
        "    calculates pp for touchscreen / touch devices. can \n"
        "    also be specified as mod TD\n"
        "\n"
        "[n]speed\n"
        "    override speed stars. "
        "useful for maps with incorrect star rating\n"
        "    default: uses computed speed stars\n"
        "    example: 3.5speed\n"
        "\n"
    );

    info(
        "[n]aim\n"
        "    override aim stars. "
        "useful for maps with incorrect star rating\n"
        "    default: uses computed aim stars\n"
        "    example: 2.4aim\n"
    );
}

#define output_sig(name) \
void name(int32_t result, struct beatmap* map, \
    struct beatmap_stats* mapstats, char const* mods_str, \
    struct diff_calc* stars, struct pp_params* params, \
    struct pp_calc* pp)

typedef output_sig(fnoutput);

/* ------------------------------------------------------------- */
/* null output                                                   */

/* stdout must be left alone, outputting to stderr is fine tho */
internalfn output_sig(output_null)
{
    (void)result; (void)map; (void)mapstats; (void)mods_str;
    (void)stars; (void)params; (void)pp;
}

#ifndef OPPAI_NOTEXT
/* ------------------------------------------------------------- */
/* text output                                                   */

#define twodec(x) (round_oppai((x) * 100.0) / 100.0)

internalfn
output_sig(output_text)
{
    uint16_t total_objs;

    if (result < 0) {
        puts(errstr(result));
        return;
    }

    printf("%s - %s ", map->artist, map->title);

    if (strcmp(map->artist, map->artist_unicode) ||
        strcmp(map->title, map->title_unicode))
    {
        printf("(%s - %s) ", map->artist_unicode,
            map->title_unicode);
    }

    printf("[%s] mapped by %s ", map->version, map->creator);
    puts("\n");

    if (mods_str) {
        printf("+%s ", mods_str);
    }

    mapstats->ar = (float)twodec(mapstats->ar);
    mapstats->od = (float)twodec(mapstats->od);
    mapstats->cs = (float)twodec(mapstats->cs);
    mapstats->hp = (float)twodec(mapstats->hp);
    stars->total = twodec(stars->total);
    stars->aim = twodec(stars->aim);
    stars->speed = twodec(stars->speed);

    printf("AR%g OD%g ", mapstats->ar, mapstats->od);

    if (map->mode == MODE_STD) {
        printf("CS%g ", mapstats->cs);
    }

    printf("HP%g\n", mapstats->hp);
    printf("300 hitwindow: %g ms\n", mapstats->odms);

    printf("%hu circles, %hu sliders, %hu spinners\n",
        map->ncircles, map->nsliders, map->nspinners);

    /* -1 because first object can't be evaluated */
    total_objs = map->ncircles + map->nsliders - 1;

    if (map->mode == MODE_STD)
    {
        printf("%d/%dx\n", params->combo, params->max_combo);

        printf("%hu spacing singletaps (%g%%)\n", stars->nsingles,
            stars->nsingles / (double)total_objs * 100.0);

        printf("%hu notes within singletap bpm threshold (%g%%)\n",
            stars->nsingles_threshold,
            stars->nsingles_threshold/(double)total_objs * 100.0);

        puts("");

        printf("%g stars (%g aim, %g speed)\n", stars->total,
            stars->aim, stars->speed);
    }
    else
    {
        printf("%d max combo\n", params->max_combo);
        printf("%g stars\n", stars->total);
    }

    printf("%g%%\n", twodec(pp->accuracy * 100));

    if (map->mode == MODE_STD) {
        printf("%g aim pp\n", twodec(pp->aim));
    }

    printf("%g speed pp\n", twodec(pp->speed));
    printf("%g acc pp\n\n", twodec(pp->acc));
    printf("%g pp\n", twodec(pp->total));
}
#endif /* OPPAI_NOTEXT */

#ifndef OPPAI_NOJSON
/* ------------------------------------------------------------- */
/* json output                                                   */

internalfn
void print_escaped_json_string(char const* str)
{
    char const* chars_to_escape = "\\\"";
    char const* p;

    putchar('"');

    for (; *str; ++str)
    {
        /* escape all characters in chars_to_escape */
        for (p = chars_to_escape; *p; ++p) {
            if (*p == *str) {
                putchar('\\');
            }
        }

        putchar(*str);
    }

    putchar('"');
}

/* https://www.doc.ic.ac.uk/%7Eeedwards/compsys/float/nan.html */

internalfn
int is_inf(double b)
{
    uint64_t* p = (uint64_t*)&b;
    return *p == 0x7FF0000000000000LL ||
           *p == 0xFFF0000000000000LL;
}

internalfn
int is_nan(double b)
{
    uint64_t* p = (uint64_t*)&b;
    return
        (*p > 0x7FF0000000000000LL && *p < 0x8000000000000000LL) ||
        (*p > 0xFFF7FFFFFFFFFFFFLL && *p <= 0xFFFFFFFFFFFFFFFFLL);
}

/* json is mentally challenged and can't handle inf and nan so
   we're gonna be mathematically incorrect */
internalfn
void fix_json_dbl(double* v)
{
    if (is_inf(*v)) {
        *v = -1;
    }

    if (is_nan(*v)) {
        *v = 0;
    }
}

internalfn
output_sig(output_json)
{
    printf("{\"oppai_version\":\"" OPPAI_VERSION_STRING "\",");

    if (result < 0)
    {
        printf("\"code\":%d,", result);
        printf("\"errstr\":");
        print_escaped_json_string(errstr(result));
        printf("}");
        return;
    }

    fix_json_dbl(&pp->total);
    fix_json_dbl(&pp->aim);
    fix_json_dbl(&pp->speed);
    fix_json_dbl(&pp->accuracy);
    fix_json_dbl(&stars->total);
    fix_json_dbl(&stars->aim);
    fix_json_dbl(&stars->speed);

    printf("\"code\":200,\"errstr\":\"no error\",");

    printf("\"artist\":");
    print_escaped_json_string(map->artist);

    if (strcmp(map->artist, map->artist_unicode))
    {
        printf(",\"artist_unicode\":");
        print_escaped_json_string(map->artist_unicode);
    }

    printf(",\"title\":");
    print_escaped_json_string(map->title);

    if (strcmp(map->title, map->title_unicode))
    {
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
        "\"mods_str\":\"%s\",\"mods\":%u,"
        "\"od\":%g,\"ar\":%g,\"cs\":%g,\"hp\":%g,"
        "\"combo\":%d,\"max_combo\":%d,"
        "\"num_circles\":%hu,\"num_sliders\":%hu,"
        "\"num_spinners\":%hu,\"misses\":%hu,"
        "\"score_version\":%u,\"stars\":%.17g,"
        "\"speed_stars\":%.17g,\"aim_stars\":%.17g,"
        "\"nsingles\":%hu,\"nsingles_threshold\":%hu,"
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
#endif /* OPPAI_NOJSON */

#ifndef OPPAI_NOCSV
/* ------------------------------------------------------------- */
/* csv output                                                    */

internalfn
void print_escaped_csv_string(char const* str)
{
    char const* chars_to_escape = "\\;";
    char const* p;

    for (; *str; ++str)
    {
        /* escape all characters in chars_to_escape */
        for (p = chars_to_escape; *p; ++p) {
            if (*p == *str) {
                putchar('\\');
            }
        }

        putchar(*str);
    }
}

internalfn
output_sig(output_csv)
{
    printf("oppai_version;" OPPAI_VERSION_STRING "\n");

    if (result < 0)
    {
        printf("code;%d\nerrstr;", result);
        print_escaped_csv_string(errstr(result));
        return;
    }

    printf("code;200\nerrstr;no error\n");

    printf("artist;");
    print_escaped_csv_string(map->artist);
    puts("");

    if (strcmp(map->artist, map->artist_unicode))
    {
        printf("artist_unicode;");
        print_escaped_csv_string(map->artist_unicode);
        puts("");
    }

    printf("title;");
    print_escaped_csv_string(map->title);
    puts("");

    if (strcmp(map->title, map->title_unicode))
    {
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
        "mods_str;%s\nmods;%u\nod;%g\nar;%g\ncs;%g\nhp;%g\n"
        "combo;%d\nmax_combo;%d\nnum_circles;%hu\n"
        "num_sliders;%hu\nnum_spinners;%hu\nmisses;%hu\n"
        "score_version;%u\nstars;%.17g\nspeed_stars;%.17g\n"
        "aim_stars;%.17g\nnsingles;%hu\nnsingles_threshold;%hu\n"
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
#endif /* OPPAI_NOCSV */

#ifndef OPPAI_NOBINARY
/* ------------------------------------------------------------- */
/* binary output                                                 */

internalfn
void write1(uint8_t v) {
    fwrite(&v, 1, 1, stdout);
}

internalfn
void write2(uint16_t v)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)(v >> 8);
    fwrite(buf, 1, 2, stdout);
}

internalfn
void write4(uint32_t v)
{
    uint8_t buf[4];

    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)((v >> 8) & 0xFF);
    buf[2] = (uint8_t)((v >> 16) & 0xFF);
    buf[3] = (uint8_t)((v >> 24) & 0xFF);

    fwrite(buf, 1, 4, stdout);
}

internalfn
void write_flt(float f)
{
    uint32_t* p = (uint32_t*)&f;
    write4(*p);
}

internalfn
void write_dbl(double d)
{
    uint64_t* p = (uint64_t*)&d;
    uint64_t v = *p;
    uint8_t buf[8];

    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)((v >> 8) & 0xFF);
    buf[2] = (uint8_t)((v >> 16) & 0xFF);
    buf[3] = (uint8_t)((v >> 24) & 0xFF);
    buf[4] = (uint8_t)((v >> 32) & 0xFF);
    buf[5] = (uint8_t)((v >> 40) & 0xFF);
    buf[6] = (uint8_t)((v >> 48) & 0xFF);
    buf[7] = (uint8_t)((v >> 56) & 0xFF);

    fwrite(buf, 1, 8, stdout);
}

internalfn
void write_str(char const* str)
{
    uint16_t len = (uint16_t)mymin(0xFFFF, strlen(str));
    write2(len);
    printf("%s", str);
    write1(0);
}

internalfn
output_sig(output_binary)
{
    (void)mods_str;

    if (!freopen(0, "wb", stdout)) {
        perror("freopen");
        exit(1);
    }

    printf("binoppai");
    write1(OPPAI_VERSION_MAJOR);
    write1(OPPAI_VERSION_MINOR);
    write1(OPPAI_VERSION_PATCH);
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
    write_dbl(stars->total);
    write_dbl(stars->speed);
    write_dbl(stars->aim);
    write2(stars->nsingles);
    write2(0);
    write2(stars->nsingles_threshold);
    write_dbl(pp->aim);
    write_dbl(pp->speed);
    write_dbl(pp->acc);
    write_dbl(pp->total);
}
#endif /* OPPAI_NOBINARY */

#ifdef OPPAI_DEBUG
/* ------------------------------------------------------------- */
/* debug output                                                  */

output_sig(output_debug)
{
    int32_t i;

    (void)mods_str;
    (void)stars;
    (void)params;

    if (result < 0) {
        puts(errstr(result));
        return;
    }

    for (i = 0; i < map->ntiming_points; ++i)
    {
        struct timing* t = &map->timing_points[i];
        printf(
            "timing %gms %g %d\n", t->time, t->ms_per_beat,
            t->change
        );
    }

    for (i = 0; i < map->nobjects; ++i)
    {
        struct object* o = &map->objects[i];

        printf("%gs [%g %g] ", o->time / 1000.0,
            o->strains[0], o->strains[1]);

        if (o->type & OBJ_CIRCLE)
        {
            struct circle* c = (struct circle*)o->pdata;
            printf(
                "circle (%g, %g) (%g, %g)\n",
                c->pos[0], c->pos[1],
                o->normpos[0], o->normpos[1]
            );
        }

        else if (o->type & OBJ_SPINNER) {
            puts("spinner");
        }

        else if (o->type & OBJ_SLIDER)
        {
            struct slider* s = (struct slider*)o->pdata;
            printf(
                "slider (%g, %g) (%g, %g)\n",
                s->pos[0], s->pos[1],
                o->normpos[0], o->normpos[1]
            );
        }

        else {
            printf("invalid hitobject %08X\n", o->type);
            break;
        }
    }

    printf("AR%g OD%g CS%g HP%g\n", mapstats->ar, mapstats->od,
        mapstats->cs, mapstats->hp);

    printf("%hu circles, %hu sliders, %hu spinners\n",
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

struct output_module
{
    char const* name;
    fnoutput* func;
    char const* description[4];
    /* null terminated array of strings because of c90 literal
       limits */
}
const modules[] =
{
    { "null", output_null, { "no output", 0 } },
#ifndef OPPAI_NOTEXT
    { "text", output_text, { "plain text", 0 } },
#endif
#ifndef OPPAI_NOJSON
    {
        "json", output_json,
        { "a single utf-8 json object.\n" CODE_DESC, 0 }
    },
#endif
#ifndef OPPAI_NOCSV
    {
        "csv", output_csv,
        { "fieldname;value\n"
        "one value per line. ';' characters in strings will be " 
        "escaped to \"\\;\". utf-8.\n" CODE_DESC, 0 }
    },
#endif
#ifndef OPPAI_NOBINARY
    {
        "binary",
        output_binary,
        { "binary stream of values, encoded in little endian.\n"
        "negative code values indicate an error, which matches "
        "the error codes defined in oppai.c\n"
        "for an example on how to read this in C, check out "
        "examples/binary.c in oppai-ng's source\n"
        "\n"
        "doubles and floats are represented using whatever "
        "convention the host machine and compiler use. unless you "
        "are on a really exotic machine it shouldn't matter\n"
        "\n"
        "strings (str) are encoded as a 2-byte integer indicating "
        "the length in bytes, followed by the string bytes and ",
        "a null (zero) terminating byte\n"
        "\n"
        "binoppai (8-byte magic), "
        "uint8_t oppai_ver_major, uint8_t oppai_ver_minor, "
        "uint8_t oppai_ver_patch, int32_t code, "
        "str artist, str artist_utf8, str title, str title_utf8, "
        "str version, str creator, "
        "uint32_t mods_bitmask, float od, float ar, float cs, "
        "float hp, int32_t combo, int32_t max_combo, "
        "uint16_t ncircles, uint16_t nsliders, uint16_t nspinner, "
        "uint32_t score_version, double total_stars, ",
        "double speed_stars, double aim_stars, uint16_t nsingles, "
        "uint16_t deprecated (always 0), "
        "uint16_t nsingles_threshold, double aim_pp, "
        "double speed_pp, double acc_pp, double pp",
        0 }
    },
#endif /* OPPAI_NOBINARY */
#ifdef OPPAI_DEBUG
    { "debug", output_debug, { "debug output", 0 } },
#endif
};

internalfn
struct output_module const*
output_by_name(char const* name)
{
    int32_t i;

    for (i = 0; i < sizeof(modules) / sizeof(modules[0]); ++i)
    {
        if (!strcmp(modules[i].name, name)) {
            return &modules[i];
        }
    }

    return 0;
}

#ifdef OPPAI_DEBUG
internalfn
void print_memory_usage(struct parser* pa, struct diff_calc* dc)
{
    info(
        "-------------------------\n"
        "strings: %dK\n"
        "timing: %dK\n"
        "objects: %dK\n"
        "object data: %dK\n"
        "strains: %dK\n"
        "total: %dK\n"
        "-------------------------\n",
        pa ? pa->strings.size / 1000 : 0,
        pa ? pa->timing.size / 1000 : 0,
        pa ? pa->objects.size / 1000 : 0,
        pa ? pa->object_data.size / 1000 : 0,
        dc ? dc->highest_strains.size / 1000 : 0,
        (
            pa ? (pa->strings.size +
                  pa->timing.size +
                  pa->objects.size +
                  pa->object_data.size) : 0 +
            dc ? dc->highest_strains.size : 0
        ) / 1000
    );
}
#else
#define print_memory_usage(x, y)
#endif /* OPPAI_DEBUG */

internalfn
int cmpsuffix(char const* str, char const* suffix)
{
    int32_t sufflen = (int32_t)mymin(strlen(str), strlen(suffix));
    return strcmp(str + strlen(str) - sufflen, suffix);
}

internalfn
char lowercase(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }

    return c;
}

internalfn
char uppercase(char c)
{
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }

    return c;
}

internalfn
int strcmp_nc(char const* a, char const* b)
{
    for (;; ++a, ++b)
    {
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
int main(int argc, char* argv[])
{
    FILE* f;
    int i;
    int32_t result;

    struct parser* pstate = 0;
    struct diff_calc stars;
    struct beatmap map;
    struct beatmap_stats mapstats;
    struct pp_params params;
    struct pp_calc pp;
    struct output_module const* m;

    char* output_name = "text";
    char* mods_str = 0;
    uint32_t mods = MODS_NOMOD;
    double acc_percent = 100.0;
    int use_percent = 0;

    uint32_t overrides = 0;
#   define OVERRIDE_AR ((uint32_t)1<<0)
#   define OVERRIDE_OD ((uint32_t)1<<1)
#   define OVERRIDE_CS ((uint32_t)1<<2)
#   define OVERRIDE_SINGLETAP_THRESHOLD ((uint32_t)1<<3)
#   define OVERRIDE_MODE ((uint32_t)1<<4)
#   define OVERRIDE_SPEED ((uint32_t)1<<5)
#   define OVERRIDE_AIM ((uint32_t)1<<6)

    float ar_override = 0, od_override = 0, cs_override = 0;
    double singletap_threshold = 125.0;
    uint32_t mode_override = MODE_STD;
    double speed_override = 0, aim_override = 0;

    /* parse arguments ----------------------------------------- */
    me = argv[0];

    if (argc < 2)
    {
        usage();
        return 1;
    }

    if (*argv[1] == '-' && strlen(argv[1]) > 1)
    {
        char* a = argv[1] + 1;

        if (!strcmp_nc(a, "version") || !strcmp_nc(a, "v"))
        {
            puts(OPPAI_VERSION_STRING);
            return 0;
        }
    }

    pp_init(&params);

    for (i = 2; i < argc; ++i)
    {
        char* a = argv[i];
        char* p;
        int iswhite = 1;

        for (p = a; *p; ++p)
        {
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

        if (*a == '-' && a[1] == 'o')
        {
            output_name = a + 2;

            if (!strcmp(output_name, "?"))
            {
                int32_t i;
                int32_t nmodules =
                    sizeof(modules) / sizeof(modules[0]);

                for (i = 0; i < nmodules; ++i)
                {
                    char const* const* d = modules[i].description;
                    puts(modules[i].name);
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
            sscanf(a + 3, "%lf", &singletap_threshold) == 1)
        {
            singletap_threshold =
                (60000.0 / singletap_threshold) / 2.0;
            overrides |= OVERRIDE_SINGLETAP_THRESHOLD;
            continue;
        }

        if (!cmpsuffix(a, "%") &&
            sscanf(a, "%lf", &acc_percent) == 1)
        {
            use_percent = 1;
            continue;
        }

        if (!cmpsuffix(a, "x100") &&
            sscanf(a, "%hu", &params.n100) == 1)
        {
            continue;
        }

        if (!cmpsuffix(a, "x50") &&
            sscanf(a, "%hu", &params.n50) == 1)
        {
            continue;
        }

        if (!cmpsuffix(a, "xm") || !cmpsuffix(a, "xmiss") ||
            !cmpsuffix(a, "m"))
        {
            if (sscanf(a, "%hu", &params.nmiss) == 1) {
                continue;
            }
        }

        if (!cmpsuffix(a, "x") &&
            sscanf(a, "%d", &params.combo) == 1)
        {
            continue;
        }

        if (sscanf(a, "scorev%u", &params.score_version)) {
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

        if (sscanf(a, "-m%u", &mode_override) == 1) {
            overrides |= OVERRIDE_MODE;
            continue;
        }

        if (!strcmp(a, "-taiko")) {
            overrides |= OVERRIDE_MODE;
            mode_override = MODE_TAIKO;
            continue;
        }

        if (!cmpsuffix(a, "speed") &&
            sscanf(a, "%lf", &speed_override) == 1)
        {
            overrides |= OVERRIDE_SPEED;
            continue;
        }

        if (!cmpsuffix(a, "aim") &&
            sscanf(a, "%lf", &aim_override) == 1)
        {
            overrides |= OVERRIDE_AIM;
            continue;
        }

        if (!strcmp(a, "-touch")) {
            mods |= MODS_TOUCH_DEVICE;
            continue;
        }

        /* this should be last because it uppercase's the string */
        if (*a == '+')
        {
            mods_str = a + 1;

            for (p = mods_str; *p; ++p) {
                *p = uppercase(*p);
            }

#           define m(mod) \
            if (!strncmp(p, #mod, strlen(#mod))) { \
                mods |= MODS_##mod; \
                p += strlen(#mod); \
                continue; \
            } \

            for (p = mods_str; *p;)
            {
                m(NF) m(EZ) m(HD) m(HR) m(DT) m(HT) m(NC) m(FL)
                m(SO) m(NOMOD)

                if (!strncmp(p, "TD", 2)) {
                    mods |= MODS_TOUCH_DEVICE;
                    p += 2;
                    continue;
                }

                ++p;
            }
#undef m

            continue;
        }

        info(">%s\n", a);
        result = ERR_SYNTAX;
        goto output;
    }

    /* parse beatmap ------------------------------------------- */
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

    pstate = (struct parser*)malloc(sizeof(struct parser));
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
    if (result < 0)
    {
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

    /* diff calc ----------------------------------------------- */
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

    if (overrides & OVERRIDE_SPEED)
    {
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

    if (use_percent)
    {
        switch (map.mode)
        {
        case MODE_STD:
            acc_round(acc_percent, (uint16_t)map.nobjects,
                params.nmiss, &params.n300, &params.n100,
                &params.n50);
            break;

        case MODE_TAIKO:
        {
            int32_t taiko_max_combo = b_max_combo(&map);

            if (taiko_max_combo < 0) {
                result = taiko_max_combo;
                goto output;
            }

            params.max_combo = (uint16_t)taiko_max_combo;

            taiko_acc_round(acc_percent, (uint16_t)taiko_max_combo,
                params.nmiss, &params.n300, &params.n100);
            break;
        }
        }
    }

    result = b_ppv2p(&map, &pp, &params);

    /* output -------------------------------------------------- */
output:
    print_memory_usage(pstate, &stars);

    m = output_by_name(output_name);
    if (!m)
    {
        info(
            "output module '%s' does not exist. "
            "check 'oppai - -o?'\n",
            output_name
        );
        return 1;
    }

    m->func(result, &map, &mapstats, mods_str, &stars,
        &params, &pp);

    return result < 0;
}

