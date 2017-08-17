/* this is free and unencumbered software released into the
   public domain.

   refer to the attached UNLICENSE or http://unlicense.org/
   ----------------------------------------------------------------
   this is a pure C89 rewrite of oppai, my osu! difficulty and pp
   calculator. it's meant to be tiny and easy to include in your
   projects without pulling in dependencies.
   ----------------------------------------------------------------
   usage:

   #define OPPAI_IMPLEMENTATION and include this file.
   if multiple compilation units need to include oppai, only define
   OPPAI_IMPLEMENTATION in one of them

   see the interface below this comment for detailed documentation
   ----------------------------------------------------------------
   #define OPPAI_IMPLEMENTATION
   #include "../oppai.c"

   int main()
   {
       struct parser pstate;
       struct beatmap map;

       uint32_t mods;
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
   ----------------------------------------------------------------
   $ gcc test.c
   $ cat /path/to/file.osu | ./a.out
   ...                                                           */

/* all structs are not thread safe, use one parser, diff_calc, etc
   instance per thread if you need to process maps in parallel */

#define OPPAI_VERSION_MAJOR 1
#define OPPAI_VERSION_MINOR 0
#define OPPAI_VERSION_PATCH 11

/* if your compiler doesn't have stdint, define this */
#ifdef OPPAI_NOSTDINT
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

#include <stdio.h> /* FILE */

#define round_oppai(x) floor((x) + 0.5)
#define mymin(a, b) ((a) < (b) ? (a) : (b))
#define mymax(a, b) ((a) > (b) ? (a) : (b))

/* ------------------------------------------------------------- */
/* errors                                                        */

/* all functions that return int32_t can return errors in the form
   of a negative value. check if the return value is < 0 and call
   errstr to get the error message */

#define ERR_MORE           (-1)
#define ERR_SYNTAX         (-2)
#define ERR_TRUNCATED      (-3)
#define ERR_NOTIMPLEMENTED (-4)
#define ERR_IO             (-5)
#define ERR_FORMAT         (-6)
#define ERR_OOM            (-7)

char const* errstr(int32_t err);

/* ------------------------------------------------------------- */
/* beatmap utils                                                 */

/* object types used in struct object */
#define OBJ_CIRCLE (1<<0)
#define OBJ_SLIDER (1<<1)
#define OBJ_SPINNER (1<<3)

/* data about a single hitobject */
struct object
{
    double time; /* milliseconds */
    uint8_t type;

    /* should be casted to struct circle or slider based on type
       should only be set/used when all parsing is done */
    void* pdata;

    /* points to one of the struct below inside object_data_mem
       only used internally by the parser */
    int32_t data_off;

    /* only used by d_calc */
    double normpos[2];
    double strains[2];
    int is_single; /* 1 if diff calc sees this as a singletap */

    /* note: it's a bit of a waste of memory to put diff calc
       stuff here, but it saves a complete allocation of a new
       diffcalc hitobject stack which is much better */
};

struct circle {
    double pos[2];
};

struct slider
{
    double pos[2];
    double distance;
    uint32_t repetitions;
};

/* timing point */
struct timing
{
    double time; /* milliseconds */
    double ms_per_beat;
    int change; /* if 0, ms_per_beat is -100.0 * sv_multiplier */
};

#define MODE_STD 0

struct beatmap
{
    int32_t mode;

    char* title;
    char* title_unicode;
    char* artist;
    char* artist_unicode;
    char* creator;
    char* version;

    struct object* objects;
    int32_t nobjects;
    struct timing* timing_points;
    int32_t ntiming_points;

    uint16_t ncircles, nsliders, nspinners;
    float hp, cs, od, ar, sv;
    float tick_rate;
};

#ifndef OPPAI_NOPARSER
/* ------------------------------------------------------------- */
/* beatmap parser                                                */

/* contiguous self-resizing stack of memory, used internally by
   the parser and diff calculator.
   pushes are byte aligned */
struct memstack
{
    uint8_t* buf;
    int32_t top;
    int32_t size;
};

/* non-null terminated string, used internally for parsing */
struct slice
{
    char* start;
    char* end; /* *(end - 1) is the last character */
};

/* beatmap parser's state */
struct parser
{
    /* if a parsing error occurs last line and portion of the line
       that was being parsed are stored in these two slices */
    struct slice lastpos;
    struct slice lastline;
    /* you probably don't care about the other fields */

    /* used to buffer data from the beatmap file */
    char buf[65536];

    int32_t title; /* offsets into the memory stack */
    int32_t title_unicode;
    int32_t artist;
    int32_t artist_unicode;
    int32_t creator;
    int32_t version;

    char section[64]; /* current section */

    struct memstack strings; /* non-critical strings */
    struct memstack timing;
    struct memstack objects;
    struct memstack object_data;

    struct beatmap* b;
};

int32_t p_init(struct parser* pa);
void p_free(struct parser* pa);

/* parses a beatmap file and stores results in b.

   NOTE: b is valid only as long as pa is not deallocated or
         reused. if you need to store maps for longer than the
         parser's lifetime, you will have to manually copy.

   returns n. bytes processed on success, < 0 on failure */
int32_t p_map(struct parser* pa, struct beatmap* b, FILE* f);

#endif /* OPPAI_NOPARSER */

/* ------------------------------------------------------------- */
/* mods utils                                                    */

/* mod bits for mods_apply */
#define MODS_NOMOD 0
#define MODS_NF ((uint32_t)1<<0)
#define MODS_EZ ((uint32_t)1<<1)
#define MODS_HD ((uint32_t)1<<3)
#define MODS_HR ((uint32_t)1<<4)
#define MODS_DT ((uint32_t)1<<6)
#define MODS_HT ((uint32_t)1<<8)
#define MODS_NC ((uint32_t)1<<9)
#define MODS_FL ((uint32_t)1<<10)
#define MODS_SO ((uint32_t)1<<12)
#define MODS_SPEED_CHANGING (MODS_DT | MODS_HT | MODS_NC)
#define MODS_MAP_CHANGING (MODS_HR | MODS_EZ | MODS_SPEED_CHANGING)

/* beatmap stats after applying mods to them */
struct beatmap_stats
{
    float ar, od, cs, hp;
    float speed; /* multiplier */
};

/* flags bits for mods_apply */
#define APPLY_AR ((uint32_t)1<<0)
#define APPLY_OD ((uint32_t)1<<1)
#define APPLY_CS ((uint32_t)1<<2)
#define APPLY_HP ((uint32_t)1<<3)
#define APPLY_ALL (~(uint32_t)0)

/* calculates beatmap stats with mods applied. s should initially
   contain the base stats. flags specifies which stats are touched.
   initial speed will always be automatically set to 1

   example:

       struct beatmap_stats s;
       s.ar = 9;
       mods_apply(MODS_DT, &s, APPLY_AR);
       // s.ar is now 10.33, s.speed is now 1.5
*/
void mods_apply(uint32_t mods,
    struct beatmap_stats* s, uint32_t flags);

#ifndef OPPAI_NODIFFCALC
/* ------------------------------------------------------------- */
/* diff calc                                                     */

/* difficulty calculation state. just like with the parser, each
   instance can be re-used in subsequent calls to d_calc */
struct diff_calc
{
    struct memstack highest_strains;
    struct beatmap* b;

    /* set this to the milliseconds interval for the maximum bpm
       you consider singletappable. defaults to 125 = 240 bpm 1/2.
       ((60000 / 240) / 2) */
    double singletap_threshold;

    /* calls to d_calc will store results here */
    double total;
    double aim;
    double speed;
    uint16_t nsingles;
    uint16_t nsingles_threshold;
};

int32_t d_init(struct diff_calc* d);
void d_free(struct diff_calc* d);

int32_t d_calc(struct diff_calc* d,
    struct beatmap* b, uint32_t mods);

#endif /* OPPAI_NODIFFCALC */

#ifndef OPPAI_NOPP
/* ------------------------------------------------------------- */
/* pp calc                                                       */

struct pp_calc
{
    /* ppv2 will store results here */
    double total, aim, speed, acc;
    double accuracy; /* 0.0 - 1.0 */
};

/* default scoring system used by ppv2() and ppv2p() */
#define PP_DEFAULT_SCORING 1

/* simplest possible call, calculates ppv2 for SS */
int32_t ppv2(struct pp_calc* pp, uint32_t mode, double aim,
    double speed, float base_ar, float base_od, int32_t max_combo,
    uint16_t nsliders, uint16_t ncircles, uint16_t nobjects,
    uint32_t mods);

/* parameters for ppv2p */
struct pp_params
{
    /* required parameters */
    double aim, speed;
    float base_ar, base_od;
    int32_t max_combo;
    uint16_t nsliders; /* required for scorev1 only */
    uint16_t ncircles; /* ^ */
    uint16_t nobjects;

    /* optional parameters */
    uint32_t mode; /* defaults to MODE_STD */
    uint32_t mods; /* defaults to MODS_NOMOD */
    int32_t combo; /* defaults to FC */
    uint16_t n300, n100, n50; /* defaults to SS */
    uint16_t nmiss; /* defaults to 0 */
    uint32_t score_version; /* defaults to PP_DEFAULT_SCORING */
};

/* initialize struct pp_params with the default values.
   required values are left untouched */
void pp_init(struct pp_params* p);

/* calculate ppv2 with advanced parameters, see struct pp_params */
int32_t ppv2p(struct pp_calc* pp, struct pp_params* p);

/* same as ppv2p but fills params automatically with the map's
   base_ar, base_od, max_combo, nsliders, ncircles, nobjects
   so you only need to provide aim and speed */
int32_t b_ppv2p(struct beatmap* map, struct pp_calc* pp,
    struct pp_params* p);

/* same as ppv2 but fills params like b_ppv2p */
int32_t b_ppv2(struct beatmap* map, struct pp_calc* pp,
    double aim, double speed, uint32_t mods);

#endif /* OPPAI_NOPP */

/* calculate accuracy (0.0 - 1.0) */
double acc_calc(uint16_t n300, uint16_t n100, uint16_t n50,
    uint16_t misses);

/* round percent accuracy to closest amount of 300s, 100s, 50s */
void acc_round(double acc_percent, uint16_t nobjects,
    uint16_t nmisses, uint16_t* n300, uint16_t* n100,
    uint16_t* n50);

/* ############################################################# */
/* ################### END OF THE INTERFACE #################### */
/* ############################################################# */

#ifdef OPPAI_IMPLEMENTATION
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define internalfn static
#define global static

/* ------------------------------------------------------------- */
/* error utils                                                   */

internalfn
int info(char const* fmt, ...)
{
    int res;

    va_list va;
    va_start(va, fmt);
    res = vfprintf(stderr, fmt, va);
    va_end(va);

    return res;
}

char const* errstr(int32_t err)
{
    switch (err)
    {
        case ERR_MORE:   return "call me again with more data";
        case ERR_SYNTAX: return "syntax error";

        case ERR_TRUNCATED:
            return "data was truncated, possibly because it was "
                   "too big for the internal buffers";

        case ERR_NOTIMPLEMENTED:
            return "requested a feature that isn't implemented";

        case ERR_IO:     return "i/o error";
        case ERR_FORMAT: return "invalid input format";
        case ERR_OOM:    return "out of memory";
    }

    info("W: got unknown error %d\n", err);
    return "unknown error";
}

#ifndef OPPAI_NODIFFCALC
/* ------------------------------------------------------------- */
/* math                                                          */

/* dst = a - b */
internalfn
void v2f_sub(double* dst, double* a, double* b)
{
    dst[0] = a[0] - b[0];
    dst[1] = a[1] - b[1];
}

internalfn
double v2f_len(double* v) {
    return sqrt(v[0] * v[0] + v[1] * v[1]);
}
#endif /* OPPAI_NODIFFCALC */

#ifndef OPPAI_NOPARSER
/* ------------------------------------------------------------- */
/* string utils                                                  */

internalfn
int whitespace(char c)
{
    switch (c)
    {
        case '\r':
        case '\n':
        case '\t':
        case ' ':
            return 1;
    }

    return 0;
}

internalfn
int32_t slice_write(struct slice const* s, FILE* f) {
    return (int32_t)fwrite(s->start, 1, s->end - s->start, f);
}

internalfn
int slice_whitespace(struct slice* s)
{
    char const* p = s->start;

    for (; p < s->end; ++p)
    {
        if (!whitespace(*p)) {
            return 0;
        }
    }

    return 1;
}

internalfn              /* trims leading and trailing whitespace */
void slice_trim(struct slice* s)
{
    for (; s->start < s->end && whitespace(*s->start); ++s->start);
    for (; s->end > s->start && whitespace(*(s->end-1)); --s->end);
}

internalfn
int slice_cmp(struct slice const* s, char const* str)
{
    int32_t len = (int32_t)strlen(str);
    int32_t s_len = (int32_t)(s->end - s->start);

    if (len < s_len) {
        return -1;
    }

    if (len > s_len) {
        return 1;
    }

    return strncmp(s->start, str, len);
}

internalfn
int32_t slice_len(struct slice const* s) {
    return (int32_t)(s->end - s->start);
}

/* splits s at any of the separators in separator_list and stores
   pointers to the strings in arr.
   returns the number of elements written to arr.
   if more elements than nmax are found, err is set to
   ERR_TRUNCATED */
internalfn
int32_t slice_split(struct slice const* s,
    char const* separator_list, struct slice* arr, int32_t nmax,
    int32_t* err)
{
    int32_t res = 0;
    char* p = s->start;
    char* pprev = p;

    if (!nmax) {
        return 0;
    }

    if (!*separator_list)
    {
        *arr = *s;
        return 1;
    }

    for (; p <= s->end; ++p)
    {
        char const* sep = separator_list;

        for (; *sep; ++sep)
        {
            if (p >= s->end || *sep == *p)
            {
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
#endif /* OPPAI_NOPARSER */

#if !defined(OPPAI_NOPARSER) || !defined(OPPAI_NODIFFCALC)
/* ------------------------------------------------------------- */
/* memstack                                                      */

internalfn
void* m_init(struct memstack* m, int32_t initial_size)
{
    memset(m, 0, sizeof(struct memstack));
    m->buf = (uint8_t*)malloc(initial_size);
    if (m->buf) {
        m->size = initial_size;
    }
    return m->buf;
}

internalfn
void m_free(struct memstack* m)
{
    free(m->buf);
    memset(m, 0, sizeof(struct memstack));
}

internalfn
void* m_reserve(struct memstack* m, int32_t nbytes)
{
    void* res;
    int32_t avail;

    avail = m->size - m->top;

    while (avail < nbytes)
    {
        int32_t newsize;
        void* newbuf;

        newsize = m->size * 2;
        newbuf = realloc(m->buf, newsize);
        if (newbuf) {
            goto allocated;
        }

        newsize = (int32_t)(m->size * 1.5f);
        newbuf = realloc(m->buf, newsize);
        if (newbuf) {
            goto allocated;
        }

        info(
            "W: you are extremely low on memory, you should "
            "probably close some stuff\n"
        );

        newsize = m->size + nbytes - avail;
        newbuf = realloc(m->buf, newsize);

        if (!newbuf) {
            /* out of memory :( */
            return 0;
        }

allocated:
        m->buf = (uint8_t*)newbuf;
        m->size = newsize;

        avail = m->size - m->top;
    }

    res = m->buf + m->top;
    m->top += nbytes;

    return res;
}

internalfn
void* m_push(struct memstack* m, void const* p, int32_t nbytes)
{
    void* res = m_reserve(m, nbytes);
    if (res) {
        memcpy(res, p, nbytes);
    }
    return res;
}

internalfn
void* m_push_slice(struct memstack* m, struct slice* s) {
    return m_push(m, s->start, (int32_t)(s->end - s->start));
}

internalfn
void* m_at(struct memstack* m, int32_t off)
{
    if (off < 0 || off >= m->top) {
        return 0;
    }

    return m->buf + off;
}
#endif /* !defined(OPPAI_NOPARSER) || !defined(OPPAI_NODIFFCALC) */

/* ------------------------------------------------------------- */
/* mods                                                          */

#define OD0_MS 79.5f
#define OD10_MS 19.5f
#define AR0_MS 1800.f
#define AR5_MS 1200.f
#define AR10_MS 450.f

#define OD_MS_STEP 6.f
#define AR_MS_STEP1 120.f /* ar0-5 */
#define AR_MS_STEP2 150.f /* ar5-10 */

void mods_apply(uint32_t mods,
    struct beatmap_stats* s, uint32_t flags)
{
    float od_ar_hp_multiplier;

    s->speed = 1.f;

    if (!(mods & MODS_MAP_CHANGING)) {
        return;
    }

    /* speed */
    if (mods & (MODS_DT | MODS_NC)) {
        s->speed *= 1.5f;
    }

    if (mods & MODS_HT) {
        s->speed *= 0.75f;
    }

    if (!flags) {
        return;
    }

    /* global multipliers */
    od_ar_hp_multiplier = 1.f;

    if (mods & MODS_HR) {
        od_ar_hp_multiplier *= 1.4f;
    }

    if (mods & MODS_EZ) {
        od_ar_hp_multiplier *= 0.5f;
    }

    /* od */
    if (flags & APPLY_OD)
    {
        float odms;

        s->od *= od_ar_hp_multiplier;
        odms = OD0_MS - (float)ceil(OD_MS_STEP * s->od);

        /* stats must be capped to 0-10 before HT/DT which brings
           them to a range of -4.42 to 11.08 for OD and -5 to 11
           for AR */
        odms = mymin(OD0_MS, mymax(OD10_MS, odms));
        odms /= s->speed; /* apply speed-changing mods */
        s->od = (OD0_MS - odms) / OD_MS_STEP; /* back to stat */
    }

    /* ar */
    if (flags & APPLY_AR)
    {
        float arms;

        s->ar *= od_ar_hp_multiplier;

        /* convert AR into its milliseconds value */
        arms = s->ar <= 5
            ? (AR0_MS - AR_MS_STEP1 *  s->ar     )
            : (AR5_MS - AR_MS_STEP2 * (s->ar - 5));

        arms = mymin(AR0_MS, mymax(AR10_MS, arms));
        arms /= s->speed;

        s->ar = arms > AR5_MS
            ? (       (AR0_MS - arms) / AR_MS_STEP1)
            : (5.0f + (AR5_MS - arms) / AR_MS_STEP2);
    }

    /* cs */
    if (flags & APPLY_CS)
    {
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
}

/* ------------------------------------------------------------- */
/* beatmap                                                       */

int32_t b_max_combo(struct beatmap* b)
{
    int32_t res = b->nobjects;
    int32_t i;

    double infinity = strtod("inf", 0);
    double tnext = -infinity;
    int32_t tindex = -1;
    double px_per_beat = infinity;

    /* slider ticks */
    for (i = 0; i < b->nobjects; ++i)
    {
        struct object* o = &b->objects[i];
        struct slider* sl;
        int32_t ticks;
        double num_beats;

        if (!(o->type & OBJ_SLIDER)) {
            continue;
        }

        /* keep track of the current timing point without searching
           the entire array for every object.
           should be a nice performance boost */
        while (o->time >= tnext)
        {
            double sv_multiplier;
            struct timing* t;

            ++tindex;

            if (b->ntiming_points > tindex + 1) {
                tnext = b->timing_points[tindex + 1].time;
            } else {
                tnext = infinity;
            }

            t = &b->timing_points[tindex];

            sv_multiplier = 1.0;
            if (!t->change && t->ms_per_beat < 0) {
                sv_multiplier = -100.0 / t->ms_per_beat;
            }

            px_per_beat = b->sv * 100.0 * sv_multiplier;
        }

        sl = (struct slider*)o->pdata;
        num_beats = (sl->distance * sl->repetitions) / px_per_beat;

        /* sliders get 2 + ticks combo (head, tail and ticks)
           each repetition adds an extra combo and an extra set of
           ticks

           calculate the number of slider ticks for one repetition
           ---
           example: a 3.75 beats slider at 1x tick rate will go:
           beat0 (head), beat1 (tick), beat2(tick), beat3(tick),
           beat3.75(tail)
           so all we have to do is ceil the number of beats and
           subtract 1 to take out the tail
           ---
           the -.1 is there to prevent ceil from ceiling whole
           values like 1.0 to 2.0 randomly
           TODO: make this consistent (maybe check lazer source) */

        ticks = (int32_t)
            ceil((num_beats - 0.1) /
            sl->repetitions * b->tick_rate);

        --ticks;

        ticks *= sl->repetitions; /* account for repetitions */
        ticks += sl->repetitions + 1; /* add heads and tails */

        /* actually doesn't include first head because we already
           added it by setting res = nobjects */
        res += mymax(0, ticks - 1);
    }

    return res;
}

#ifndef OPPAI_NOPARSER
/* ------------------------------------------------------------- */
/* beatmap parser                                                */

/* sets up parser for reuse
   must have already been initialized with p_init */
internalfn
void p_reset(struct parser* pa, struct beatmap* b)
{
    memset(pa->section, 0, sizeof(pa->section));
    memset(&pa->lastpos, 0, sizeof(pa->lastpos));
    memset(&pa->lastline, 0, sizeof(pa->lastline));

    pa->title =
    pa->title_unicode =
    pa->artist =
    pa->artist_unicode =
    pa->creator =
    pa->version = -1;

    pa->strings.top =
    pa->objects.top =
    pa->timing.top =
    pa->object_data.top = 0;

    pa->b = b;

    if (b)
    {
        b->nobjects = 0;
        b->ncircles = 0;
        b->nsliders = 0;
        b->nspinners = 0;
    }
}

int32_t p_init(struct parser* pa)
{
    memset(pa, 0, sizeof(struct parser));

    p_reset(pa, 0);

    if (!m_init(&pa->strings, 256) ||
        !m_init(&pa->objects, sizeof(struct object) * 128)||
        !m_init(&pa->timing, sizeof(struct timing) * 8)||
        !m_init(&pa->object_data, sizeof(double) * 256))
    {
        return ERR_OOM;
    }

    return 0;
}

void p_free(struct parser* pa)
{
    m_free(&pa->strings);
    m_free(&pa->objects);
    m_free(&pa->timing);
    m_free(&pa->object_data);
}

/* note: do not store pointers to memstack objects as they can be
         realloc'd as more stuff is pushed */

/* these macros define helpers to access and push to mem stacks */

#define SIMPLE_INDEXED(t, stack) \
internalfn \
struct t* p_get_##stack(struct parser* p) { \
    return (struct t*)p->stack.buf; \
} \
\
internalfn \
int32_t p_n##stack(struct parser* p) { \
    return p->stack.top / sizeof(struct t); \
}

SIMPLE_INDEXED(object, objects)
SIMPLE_INDEXED(timing, timing)

#define SIMPLE_PUSH(t, stack) \
internalfn \
struct t* p_push_##t(struct parser* p, struct t* x) { \
    return (struct t*)m_push(&p->stack, x, sizeof(struct t)); \
}

#define SIMPLE_AT(t, stack) \
internalfn \
struct t* p_##t##_at(struct parser* p, int32_t off) { \
    return (struct t*)m_at(&p->object_data, off); \
}

SIMPLE_PUSH(object, objects)
SIMPLE_PUSH(timing, timing)
SIMPLE_PUSH(circle, object_data)
SIMPLE_PUSH(slider, object_data)

internalfn
char* p_strings_at(struct parser* p, int32_t off) {
    return (char*)m_at(&p->strings, off);
}

/* consume functions return the number of chars or < 0 on err
   the destination slice is left untouched if there are errors

   NOTE: comments in beatmaps can only be an entire line because
         some properties such as author can contain // */

/* evil hack to set lastpos in one statement */
#define parse_err(e, lastpos_) \
        pa->lastpos = (lastpos_), \
        ERR_##e

/* consume until any of the characters in separators is found */
internalfn
int32_t consume_until(struct parser* pa, struct slice const* s,
    char const* separators, struct slice* dst)
{
    char* p = s->start;

    for (; p < s->end; ++p)
    {
        char const* sep;

        for (sep = separators; *sep; ++sep)
        {
            if (*p == *sep)
            {
                dst->start = s->start;
                dst->end = p;
                return (int32_t)(p - s->start);
            }
        }
    }

    return parse_err(MORE, *s);
}

/* all parse_* functions expect s to be a single line and trimmed

   if the return type is int32_t, they return n bytes consumed
   if the return type is int, they will return zero on success

   on errors, parse_* functions return < 0 error codes */

#define print_line(line) \
    info("in line: "), \
    slice_write((line), stderr), \
    info("\n")

/* [name] */
internalfn
int32_t p_section_name(struct parser* pa,
    struct slice const* s, struct slice* name)
{
    int32_t n;
    struct slice p = *s;

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

    return (int32_t)(p.start - s->start);
}

/* name: value
   results are trimmed */
internalfn
int32_t p_property(struct parser* pa, struct slice const* s,
    struct slice* name, struct slice* value)
{
    int32_t n;
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

    return (int32_t)(s->end - s->start);
}

internalfn
char* skip_bom(char* buf)
{
    char* p = buf;
    uint8_t const utf8_bom[] = { 0xEF, 0xBB, 0xBF };

    if (!memcmp(buf, utf8_bom, sizeof(utf8_bom))) {
        p += sizeof(utf8_bom);
    }

    /* TODO: other encodings */

    return p;
}

/* checks if f is a beatmap and skips bom if present */
internalfn
int is_beatmap(FILE* f)
{
    char const* const magic = "osu file format v";
    size_t const magic_len = 17;

    char buf[64];
    char* p = buf;
    size_t toread;

    if (fread(buf, 1, 3, f) != 3) {
        perror("fread");
        return 0;
    }

    p = skip_bom(buf);

    toread = magic_len - (3 - (p - buf));
    if (fread(buf + 3, 1, toread, f) != toread) {
        perror("fread");
        return 0;
    }

    /* TODO: maybe read until next line? can't seek back because
             f could be stdin or pretty much anything */

    return strncmp(p, magic, strlen(magic)) == 0;
}

internalfn
int32_t p_metadata(struct parser* pa, struct slice* line)
{
    int32_t n;
    int32_t* dst = 0;
    struct slice name, value;

    n = p_property(pa, line, &name, &value);
    if (n < 0) {
        return n;
    }

    if (!pa->strings.buf) {
        return n;
    }

    if (!slice_cmp(&name, "Title"))
        dst = &pa->title;

    else if (!slice_cmp(&name, "TitleUnicode"))
        dst = &pa->title_unicode;

    else if (!slice_cmp(&name, "Artist"))
        dst = &pa->artist;

    else if (!slice_cmp(&name, "ArtistUnicode"))
        dst = &pa->artist_unicode;

    else if (!slice_cmp(&name, "Creator"))
        dst = &pa->creator;

    else if (!slice_cmp(&name, "Version"))
        dst = &pa->version;

    /* we have a metadata property we want, so save it */
    if (dst && *dst < 0)
    {
        char* str;
        uint8_t const zero = 0;

        str = (char*)m_push_slice(&pa->strings, &value);
        if (!str || !m_push(&pa->strings, &zero, 1))
        {
            /* since we are out of memory, free all metadata
               which is not required for diffcalc */

            pa->title = -1;
            pa->title_unicode = -1;
            pa->artist = -1;
            pa->artist_unicode = -1;
            pa->creator = -1;
            pa->version = -1;

            m_free(&pa->strings);
        }

        else {
            *dst = pa->strings.top - slice_len(&value) - 1;
        }
    }

    return n;
}

internalfn
int32_t p_general(struct parser* pa, struct slice* line)
{
    int32_t n;
    struct slice name, value;

    n = p_property(pa, line, &name, &value);
    if (n < 0) {
        return n;
    }

    if (!slice_cmp(&name, "Mode"))
    {
        sscanf(value.start, "%d", &pa->b->mode);

        if (pa->b->mode != MODE_STD) {
            return ERR_NOTIMPLEMENTED;
        }
    }

    return n;
}

internalfn
int32_t p_difficulty(struct parser* pa, struct slice* line)
{
    int32_t n;
    float* dst = 0;
    struct slice name, value;

    n = p_property(pa, line, &name, &value);
    if (n < 0) {
        return n;
    }

    if (!slice_cmp(&name, "CircleSize")) {
        dst = &pa->b->cs;
    }

    else if (!slice_cmp(&name, "OverallDifficulty")) {
        dst = &pa->b->od;
    }

    else if (!slice_cmp(&name, "ApproachRate")) {
        dst = &pa->b->ar;
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
        sscanf(value.start, "%f", dst);
        /* if it fails we can just use default values */
    }

    return n;
}


/* time, ms_per_beat, time_signature_id, sample_set_id,
   sample_bank_id, sample_volume, is_timing_change, effect_flags

   everything after ms_per_beat is optional */
internalfn
int32_t p_timing(struct parser* pa, struct slice* line)
{
    int32_t res = 0;
    int32_t n, i;
    int32_t err = 0;
    struct slice split[8];
    struct timing t;

    t.change = 1;

    n = slice_split(line, ",", split, 8, &err);
    if (err < 0)
    {
        if (err == ERR_TRUNCATED)
        {
            info("W: timing point with trailing values");
            print_line(line);
        }
        else {
            return err;
        }
    }

    if (n < 2) {
        return parse_err(SYNTAX, *line);
    }

    res = (int32_t)(split[n - 1].end - line->start);

    for (i = 0; i < n; ++i) {
        slice_trim(&split[i]);
    }

    if (sscanf(split[0].start, "%lf", &t.time) != 1) {
        return parse_err(SYNTAX, split[0]);
    }

    if (sscanf(split[1].start, "%lf", &t.ms_per_beat) != 1) {
        return parse_err(SYNTAX, split[1]);
    }

    if (n >= 7)
    {
        if (slice_len(&split[6]) < 1) {
            return parse_err(SYNTAX, split[6]);
        }

        t.change = *split[6].start != '0';
    }

    if (!p_push_timing(pa, &t)) {
        return ERR_OOM;
    }

    return res;
}

internalfn
int32_t p_objects_std(struct parser* pa, struct slice* line)
{
    struct object obj;
    int32_t nelements;
    int32_t err = 0;
    struct slice elements[11];
    uint32_t tmp_type;

    obj.is_single = 0;

    nelements = slice_split(line, ",", elements, 11, &err);
    if (err < 0)
    {
        if (err == ERR_TRUNCATED)
        {
            info("W: object with trailing values\n");
            print_line(line);
        }
        else {
            return err;
        }
    }

    if (nelements < 5) {
        return parse_err(SYNTAX, *line);
    }

    if (sscanf(elements[2].start, "%lf", &obj.time) != 1) {
        return parse_err(SYNTAX, elements[2]);
    }

    if (sscanf(elements[3].start, "%u", &tmp_type) != 1) {
        return parse_err(SYNTAX, elements[3]);
    }

    /* not in byte range, expecting 0-255 */
    if (tmp_type & 0xFFFFFF00) {
        return parse_err(SYNTAX, elements[3]);
    }

    obj.type = (uint8_t)(tmp_type & 0xFF);

    if (obj.type & OBJ_CIRCLE)
    {
        struct circle c;

        ++pa->b->ncircles;

        if (sscanf(elements[0].start, "%lf", &c.pos[0]) != 1) {
            return parse_err(SYNTAX, elements[0]);
        }

        if (sscanf(elements[1].start, "%lf", &c.pos[1]) != 1) {
            return parse_err(SYNTAX, elements[1]);
        }

        if (!p_push_circle(pa, &c)) {
            return ERR_OOM;
        }

        obj.data_off = pa->object_data.top - sizeof(struct circle);
    }

    /* ?,?,?,?,?,end_time,custom_sample_banks */
    else if (obj.type & OBJ_SPINNER)
    {
        ++pa->b->nspinners;
        obj.data_off = 0;
    }

    /* x,y,time,type,sound_type,points,repetitions,distance,
       per_node_sounds,per_node_samples,custom_sample_banks */
    else if (obj.type & OBJ_SLIDER)
    {
        struct slider sli;
        struct slice const* e = elements;

        ++pa->b->nsliders;

        memset(&sli, 0, sizeof(sli));

        if (nelements < 7) {
            return parse_err(SYNTAX, *line);
        }

        if (sscanf(e[0].start, "%lf", &sli.pos[0]) != 1) {
            return parse_err(SYNTAX, *line);
        }

        if (sscanf(e[1].start, "%lf", &sli.pos[1]) != 1) {
            return parse_err(SYNTAX, *line);
        }

        if (sscanf(e[6].start, "%u", &sli.repetitions) != 1) {
            return parse_err(SYNTAX, e[6]);
        }

        if (nelements > 7 &&
            sscanf(e[7].start, "%lf", &sli.distance) != 1)
        {
            return parse_err(SYNTAX, e[7]);
        }

        if (!p_push_slider(pa, &sli)) {
            return ERR_OOM;
        }

        obj.data_off = pa->object_data.top - sizeof(struct slider);
    }

    if (!p_push_object(pa, &obj)) {
        return ERR_OOM;
    }

    return (int32_t)(elements[nelements - 1].end - line->start);
}

internalfn
int32_t p_line(struct parser* pa, struct slice* line)
{
    int32_t n = 0;

    if (line->start >= line->end) {
        /* empty line */
        return 0;
    }

    if (slice_whitespace(line)) {
        return (int32_t)(line->end - line->start);
    }

    /* comments (according to lazer) */
    switch (*line->start)
    {
        case ' ':
        case '_':
            return (int32_t)(line->end - line->start);
    }

    /* from here on we don't care about leading or
       trailing whitespace */

    slice_trim(line);
    pa->lastline = *line;

    /* C++ style comments */
    if (!strncmp(line->start, "//", 2)) {
        return 0;
    }

    /* new section */
    if (*line->start == '[')
    {
        struct slice section;

        n = p_section_name(pa, line, &section);
        if (n < 0) {
            return n;
        }

        if (section.end - section.start >= sizeof(pa->section)) {
            return ERR_TRUNCATED;
        }

        memcpy(
            pa->section,
            section.start, section.end - section.start
        );

        pa->section[section.end - section.start] = 0;

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
        n = p_objects_std(pa, line);
    }

    return n;
}

int32_t p_map(struct parser* pa, struct beatmap* b, FILE* f)
{
    int32_t res = 0;
    char* pbuf;
    int32_t bufsize;
    int32_t n;
    int32_t nread;

    b->ar = b->od = b->cs = b->hp = 5;
    b->sv = b->tick_rate = 1;

    if (!f) {
        return ERR_IO;
    }

    if (!is_beatmap(f)) {
        return ERR_FORMAT;
    }

    p_reset(pa, b);

    /* points to free space in the buffer */
    pbuf = pa->buf;

    /* reading loop */
    for (;;)
    {
        /* complete lines in the current chunk */
        uint32_t nlines = 0;
        struct slice s; /* points to the remaining data in buf */
        int more_data;

        bufsize = (int32_t)sizeof(pa->buf) -
            (int32_t)(pbuf - pa->buf);

        nread = (int32_t)fread(pbuf, 1, bufsize, f);
        if (!nread) {
            /* eof */
            break;
        }

        more_data = !feof(f);

        s.start = pa->buf;
        s.end = pbuf + nread;

        /* parsing loop */
        for (; s.start < s.end; )
        {
            struct slice line;

            n = consume_until(pa, &s, "\n", &line);
            if (n < 0)
            {
                if (n != ERR_MORE) {
                    return n;
                }

                if (!nlines) {
                    /* line doesn't fit the entire buffer */
                    return parse_err(TRUNCATED, s);
                }

                /* we will finish reading this line later */
                if (more_data) {
                    break;
                }

                /* EOF, so we must process the remaining data
                   as a line */
                line = s;
                n = (int32_t)(s.end - s.start);
            }

            else {
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

    if (pa->title_unicode < 0) {
        pa->title_unicode = pa->title;
    }

    if (pa->artist_unicode < 0) {
        pa->artist_unicode = pa->artist;
    }

    /* copy parser values over to the beatmap struct */

#define s(x) \
    if (pa->x != -1) { \
        b->x = p_strings_at(pa, pa->x); \
    } else { \
        b->x = "(null)"; \
    }

    s(artist) s(artist_unicode) s(title) s(title_unicode)
    s(creator) s(version)
#undef s

    b->nobjects = p_nobjects(pa);
    b->objects = p_get_objects(pa);
    b->ntiming_points = p_ntiming(pa);
    b->timing_points = p_get_timing(pa);

    /* now it's safe to store pointers to the memstacks since we
       are done pushing stuff to them */
    for (n = 0; n < b->nobjects; ++n)
    {
        struct object* o = &b->objects[n];
        o->pdata = pa->object_data.buf + o->data_off;
    }

    return res;
}
#endif

#ifndef OPPAI_NODIFFCALC
/* ------------------------------------------------------------- */
/* diff calc                                                     */

/* based on tom94's osu!tp aimod and osuElements */

#define DIFF_SPEED 0
#define DIFF_AIM 1

/* how much strains decay per interval */
double const decay_base[] = { 0.3, 0.15 };

/* almost the normalized circle diameter (104) */
#define ALMOST_DIAMETER 90.0

/* arbitrary thresholds to determine when a stream is spaced enough
   that it becomes hard to alternate */
#define STREAM_SPACING 110
#define SINGLE_SPACING 125

/* used to keep speed and aim balanced between eachother */
double const weight_scaling[] = { 1400, 26.25 };

/* non-normalized diameter where the circlesize buff starts */
#define CIRCLESIZE_BUFF_TRESHOLD 30

#define STAR_SCALING_FACTOR 0.0675 /* star rating multiplier */

/* 50% of the difference between aim and speed is added to star
   rating to compensate aim only or speed only maps */
#define EXTREME_SCALING_FACTOR 0.5

#define PLAYFIELD_WIDTH 512.0 /* in osu!pixels */

/* spinners position */
double const playfield_center[] = {
    PLAYFIELD_WIDTH / 2, PLAYFIELD_WIDTH / 2
};

/* strains are calculated by analyzing the map in chunks and then
   taking the peak strains in each chunk.
   this is the length of a strain interval in milliseconds. */
#define STRAIN_STEP 400.0

/* max strains are weighted from highest to lowest, and this is
   how much the weight decays. */
#define DECAY_WEIGHT 0.9

int32_t d_init(struct diff_calc* d)
{
    memset(d, 0, sizeof(struct diff_calc));

    if (!m_init(&d->highest_strains, sizeof(double) * 600)) {
        return ERR_OOM;
    }

    d->singletap_threshold = 125; /* 240 bpm 1/2 */

    return 0;
}

void d_free(struct diff_calc* d) {
    m_free(&d->highest_strains);
}

internalfn
double d_spacing_weight(double distance,
    uint8_t type, int* is_single)
{
    switch (type)
    {
        case DIFF_SPEED:
            if (distance > SINGLE_SPACING) {
                *is_single = 1;
                return 2.5;
            }
            else if (distance > STREAM_SPACING) {
                return 1.6 + 0.9 *
                    (distance - STREAM_SPACING) /
                    (SINGLE_SPACING - STREAM_SPACING);
            }
            else if (distance > ALMOST_DIAMETER) {
                return 1.2 + 0.4 * (distance - ALMOST_DIAMETER)
                    / (STREAM_SPACING - ALMOST_DIAMETER);
            }
            else if (distance > ALMOST_DIAMETER / 2.0) {
                return 0.95 + 0.25 *
                    (distance - ALMOST_DIAMETER / 2.0) /
                    (ALMOST_DIAMETER / 2.0);
            }
            return 0.95;

        case DIFF_AIM: return pow(distance, 0.99);
    }

    return 0.0;
}

internalfn
void d_calc_strain(uint8_t type,
    struct object* o, struct object* prev, double speed_mul)
{
    double res = 0;
    double time_elapsed = (o->time - prev->time) / speed_mul;
    double decay = pow(decay_base[type], time_elapsed / 1000.0);
    double scaling = weight_scaling[type];

    /* this implementation doesn't account for sliders */
    if (o->type & (OBJ_SLIDER | OBJ_CIRCLE))
    {
        double diff[2];

        v2f_sub(diff, o->normpos, prev->normpos);
        res = d_spacing_weight(v2f_len(diff), type, &o->is_single);
        res *= scaling;
    }

    /* prevents retarded results for hit object spams */
    res /= mymax(time_elapsed, 50.0);
    o->strains[type] = prev->strains[type] * decay + res;
}

internalfn
int dbl_desc(void const* a, void const* b)
{
    double x = *(double const*)a;
    double y = *(double const*)b;

    if (x < y) {
        return 1;
    }

    if (x == y) {
        return 0;
    }

    return -1;
}

internalfn
int32_t d_calc_individual(uint8_t type,
    struct diff_calc* d, double speed_mul, double* result)
{
    uint16_t i;

    int32_t nstrains = 0;
    double* strains;

    double max_strain = 0.0;
    double difficulty = 0.0;
    double weight = 1.0;
    double interval_end = STRAIN_STEP * speed_mul;

    d->highest_strains.top = 0;

    for (i = 1; i < d->b->nobjects; ++i)
    {
        struct object* o = &d->b->objects[i];
        struct object* prev = &d->b->objects[i - 1];

        d_calc_strain(type, o, prev, speed_mul);

        /* make previous peak strain decay until the current obj */
        while (o->time > interval_end)
        {
            double decay;
            void* p;

            p = m_push(&d->highest_strains,
                &max_strain, sizeof(double));
            if (!p) {
                return ERR_OOM;
            }
            ++nstrains;

            decay = pow(decay_base[type],
                (interval_end - prev->time) / 1000.0);

            max_strain = prev->strains[type] * decay;
            interval_end += STRAIN_STEP * speed_mul;
        }

        max_strain = mymax(max_strain, o->strains[type]);
    }

    strains = (double*)d->highest_strains.buf;

    /* sort strains from highest to lowest */
    qsort(strains, nstrains, sizeof(double), dbl_desc);

    for (i = 0; i < nstrains; ++i)
    {
        difficulty += strains[i] * weight;
        weight *= DECAY_WEIGHT;
    }

    *result = difficulty;

    return 0;
}

int32_t d_calc(struct diff_calc* d, struct beatmap* b,
    uint32_t mods)
{
    int32_t i;
    int32_t res;
    double radius;
    double scaling_factor;
    struct beatmap_stats mapstats;

    if (b->mode != MODE_STD) {
        info("only osu! standard diff calc is implemented at the "
            "moment.\n");
        return ERR_NOTIMPLEMENTED;
    }

    d->b = b;

    /* apply mods and calculate circle radius at this CS */
    mapstats.cs = b->cs;
    mods_apply(mods, &mapstats, APPLY_CS);

    radius =
        (PLAYFIELD_WIDTH / 16.0) *
        (1.0 - 0.7 * ((double)mapstats.cs - 5.0) / 5.0);

    /* positions are normalized on circle radius so that we
       can calc as if everything was the same circlesize */
    scaling_factor = 52.0 / radius;

    /* cs buff (credits to osuElements, I have confirmed that
       this is indeed accurate */
    if (radius < CIRCLESIZE_BUFF_TRESHOLD)
    {
        scaling_factor *=
            1.0 +
            mymin((CIRCLESIZE_BUFF_TRESHOLD - radius), 5.0)
                / 50.0;
    }

    /* calculate normalized positions */
    for (i = 0; i < b->nobjects; ++i)
    {
        struct object* o = &d->b->objects[i];
        double const* pos;

        if (o->type & OBJ_SPINNER) {
            pos = playfield_center;
        } else {
            /* sliders also begin with pos so it's fine */
            pos = ((struct circle*)o->pdata)->pos;
        }

        o->normpos[0] = pos[0] * scaling_factor;
        o->normpos[1] = pos[1] * scaling_factor;
    }

    /* calculate speed and aim stars */
    res = d_calc_individual(DIFF_SPEED, d, mapstats.speed,
        &d->speed);
    if (res < 0) {
        return res;
    }

    res = d_calc_individual(DIFF_AIM, d, mapstats.speed, &d->aim);
    if (res < 0) {
        return res;
    }

    d->aim = sqrt(d->aim) * STAR_SCALING_FACTOR;
    d->speed = sqrt(d->speed) * STAR_SCALING_FACTOR;

    /* calculate total star rating */
    d->total = d->aim + d->speed +
        fabs(d->speed - d->aim) * EXTREME_SCALING_FACTOR;

    /* singletap stats */
    for (i = 1; i < b->nobjects; ++i)
    {
        struct object* o = &d->b->objects[i];

        if (o->is_single) {
            ++d->nsingles;
        }

        if (o->type & (OBJ_CIRCLE | OBJ_SLIDER))
        {
            struct object* prev = &d->b->objects[i - 1];
            double interval = o->time - prev->time;

            interval /= mapstats.speed;

            if (interval >= d->singletap_threshold) {
                ++d->nsingles_threshold;
            }
        }
    }

    return 0;
}
#endif

/* ------------------------------------------------------------- */
/* pp calc                                                       */

double acc_calc(uint16_t n300, uint16_t n100, uint16_t n50,
    uint16_t misses)
{
    uint16_t total_hits = n300 + n100 + n50 + misses;
    double acc = 0.f;

    if (total_hits > 0)
    {
        acc = (
            n50 * 50.0 + n100 * 100.0 + n300 * 300.0) /
            (total_hits * 300.0);
    }

    return acc;
}

void acc_round(double acc_percent, uint16_t nobjects,
    uint16_t misses, uint16_t* n300, uint16_t* n100,
    uint16_t* n50)
{
    uint16_t max300;
    double maxacc;

    misses = mymin(nobjects, misses);
    max300 = nobjects - misses;
    maxacc = acc_calc(max300, 0, 0, misses) * 100.0;
    acc_percent = mymax(0.0, mymin(maxacc, acc_percent));

    *n50 = 0;

    /* just some black magic maths from wolfram alpha */
    *n100 = (uint16_t)
        round_oppai(-3.0 * ((acc_percent * 0.01 - 1.0) *
            nobjects + misses) * 0.5);

    if (*n100 > nobjects - misses)
    {
        /* acc lower than all 100s, use 50s */
        *n100 = 0;
        *n50 = (uint16_t)
            round_oppai(-6.0 * ((acc_percent * 0.01 - 1.0) *
                nobjects + misses) * 0.2);

        *n50 = mymin(max300, *n50);
    }
    else {
        *n100 = mymin(max300, *n100);
    }

    *n300 = nobjects - *n100 - *n50 - misses;
}

#ifndef OPPAI_NOPP
/* some kind of formula to get a base pp value from stars */
internalfn
double base_pp(double stars)
{
    return pow(5.0 * mymax(1.0, stars / 0.0675) - 4.0, 3.0)
        / 100000.0;
}

internalfn
int32_t ppv2x(struct pp_calc* pp, uint32_t mode, double aim,
    double speed, float base_ar, float base_od, int32_t max_combo,
    uint16_t nsliders, uint16_t ncircles, uint16_t nobjects,
    uint32_t mods, int32_t combo, uint16_t n300, uint16_t n100,
    uint16_t n50, uint16_t nmiss, uint32_t score_version)
{
    uint16_t nspinners = nobjects - nsliders - ncircles;
    struct beatmap_stats mapstats;

    /* various pp calc multipliers */
    double nobjects_over_2k = nobjects / 2000.0;

    double length_bonus = 0.95 +
        0.4 * mymin(1.0, nobjects_over_2k) +
        (nobjects > 2000 ? log10(nobjects_over_2k) * 0.5 : 0.0);

    double miss_penality = pow(0.97, nmiss);
    double combo_break = pow(combo, 0.8) / pow(max_combo, 0.8);
    double ar_bonus;
    double final_multiplier;
    double acc_bonus, od_bonus;

    /* acc used for pp is different in scorev1 because it ignores
       sliders */
    double real_acc;

    if (mode != MODE_STD) {
        info("ppv2 is only implemented for std at the moment\n");
        return ERR_NOTIMPLEMENTED;
    }

    memset(pp, 0, sizeof(struct pp_calc));

    /* sanitize some input */
    if (max_combo <= 0) {
        info("W: max_combo <= 0, changing to 1\n");
        max_combo = 1;
    }

    /* accuracy */
    pp->accuracy = acc_calc(n300, n100, n50, nmiss);

    switch (score_version)
    {
        case 1:
            /* scorev1 ignores sliders since they are free 300s */
            /* apparently it also ignores spinners... */
            real_acc = acc_calc(n300 - nsliders - nspinners,
                n100, n50, nmiss);

            /* can go negative if we miss everything */
            real_acc = mymax(0.0, real_acc);
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

    /* ar bonus ------------------------------------------------ */
    ar_bonus = 1.0;

    /* high ar bonus */
    if (mapstats.ar > 10.33) {
        ar_bonus += 0.45 * (mapstats.ar - 10.33);
    }

    /* low ar bonus */
    else if (mapstats.ar < 8.0)
    {
        double low_ar_bonus = 0.01 * (8.0 - mapstats.ar);

        if (mods & MODS_HD) {
            low_ar_bonus *= 2.0;
        }

        ar_bonus += low_ar_bonus;
    }

    /* aim pp -------------------------------------------------- */
    pp->aim = base_pp(aim);
    pp->aim *= length_bonus;
    pp->aim *= miss_penality;
    pp->aim *= combo_break;

    pp->aim *= ar_bonus;

    /* hidden */
    if (mods & MODS_HD) {
        pp->aim *= 1.18;
    }

    /* flashlight */
    if (mods & MODS_FL) {
        pp->aim *= 1.45 * length_bonus;
    }

    /* acc bonus (bad aim can lead to bad acc, reused in speed for
       same reason) */
    acc_bonus = 0.5 + pp->accuracy / 2.0;

    /* od bonus (low od is easy to acc even with shit aim, reused
       in speed ...) */
    od_bonus = 0.98 + pow(mapstats.od, 2) / 2500.0;

    pp->aim *= acc_bonus;
    pp->aim *= od_bonus;

    /* speed pp ------------------------------------------------ */
    pp->speed = base_pp(speed);
    pp->speed *= length_bonus;
    pp->speed *= miss_penality;
    pp->speed *= combo_break;
    pp->speed *= acc_bonus;
    pp->speed *= od_bonus;

    /* acc pp -------------------------------------------------- */
    /* arbitrary values tom crafted out of trial and error */
    pp->acc =
        pow(1.52163, mapstats.od) * pow(real_acc, 24.0) * 2.83;

    /* length bonus (not the same as speed/aim length bonus) */
    pp->acc *= mymin(1.15, pow(ncircles / 1000.0, 0.3));

    /* hidden bonus */
    if (mods & MODS_HD) {
        pp->acc *= 1.02;
    }

    /* flashlight bonus */
    if (mods & MODS_FL) {
        pp->acc *= 1.02;
    }

    /* total pp ------------------------------------------------ */
    final_multiplier = 1.12;

    /* nofail */
    if (mods & MODS_NF) {
        final_multiplier *= 0.90;
    }

    /* spun-out */
    if (mods & MODS_SO) {
        final_multiplier *= 0.95;
    }

    pp->total =
        pow(
            pow(pp->aim, 1.1) +
            pow(pp->speed, 1.1) +
            pow(pp->acc, 1.1),
            1.0 / 1.1
        ) * final_multiplier;

    return 0;
}

void pp_init(struct pp_params* p)
{
    p->mode = MODE_STD;
    p->mods = MODS_NOMOD;
    p->combo = -1;
    p->n300 = 0xFFFF;
    p->n100 = p->n50 = p->nmiss = 0;
    p->score_version = PP_DEFAULT_SCORING;
}

/* should be called inside ppv2p before calling ppv2x */
internalfn
void pp_handle_default_params(struct pp_params* p)
{
    if (p->combo < 0) {
        p->combo = p->max_combo - p->nmiss;
    }

    if (p->n300 == 0xFFFF) {
        p->n300 = p->nobjects - p->n100 - p->n50 - p->nmiss;
    }
}

int32_t ppv2(struct pp_calc* pp, uint32_t mode, double aim,
    double speed, float base_ar, float base_od, int32_t max_combo,
    uint16_t nsliders, uint16_t ncircles, uint16_t nobjects,
    uint32_t mods)
{
    return ppv2x(pp, mode, aim, speed, base_ar, base_od, max_combo,
        nsliders, ncircles, nobjects, mods, max_combo,
        nobjects, 0, 0, 0, PP_DEFAULT_SCORING);
}

int32_t ppv2p(struct pp_calc* pp, struct pp_params* p)
{
    pp_handle_default_params(p);

    /* TODO: replace ppv2x with this? */
    return ppv2x(pp, p->mode, p->aim, p->speed, p->base_ar,
        p->base_od, p->max_combo, p->nsliders, p->ncircles,
        p->nobjects, p->mods, p->combo, p->n300, p->n100, p->n50,
        p->nmiss, p->score_version);
}

int32_t b_ppv2(struct beatmap* map, struct pp_calc* pp,
    double aim, double speed, uint32_t mods)
{
    int32_t max_combo = b_max_combo(map);

    return ppv2x(pp, map->mode, aim, speed, map->ar, map->od,
        max_combo, map->nsliders, map->ncircles,
        (uint16_t)map->nobjects, mods, max_combo,
        (uint16_t)map->nobjects, 0, 0, 0, PP_DEFAULT_SCORING);
}

int32_t b_ppv2p(struct beatmap* map, struct pp_calc* pp,
    struct pp_params* p)
{
    p->base_ar = map->ar;
    p->base_od = map->od;
    p->max_combo = b_max_combo(map);
    p->nsliders = map->nsliders;
    p->ncircles = map->ncircles;
    p->nobjects = (uint16_t)map->nobjects;
    p->mode = map->mode;
    pp_handle_default_params(p);

    return ppv2p(pp, p);
}
#endif /* OPPAI_NOPP */

#endif /* OPPAI_IMPLEMENTATION */

