#define OPPAI_IMPLEMENTATION
#include "../oppai.c"
#include "test_suite.c" /* defines suite */

#define ERROR_MARGIN 0.02 /* pp can be off by +- 2% */
/* margin is actually
   - 3x for < 100pp
   - 2x for 100-200pp
   - 1.5x for 200-300pp */

#ifndef OPPAI_NOCURL
#include <curl/curl.h>

static size_t wrchunk(void* p, size_t cb, size_t nmemb, void* fd) {
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

static void check_err(int32_t result)
{
    if (result < 0) {
        fprintf(stderr, "%s\n", errstr(result));
        exit(1);
    }
}

static void print_score(struct score const* s)
{
    char mods_str_buf[20];
    char* mods_str = mods_str_buf;

    strcpy(mods_str, "nomod");

#   define M(mod) \
    if (s->mods & MODS_##mod) { \
        mods_str += sprintf(mods_str, #mod); \
    } \

    M(HR) M(NC) M(HT) M(SO) M(NF) M(EZ) M(DT) M(FL) M(HD)
#   undef M

    fprintf(
        stderr,
        "%u +%s %dx %hdx300 %hdx100 %hdx50 %hdxmiss "
        "%g pp\n",
        s->id, mods_str_buf, s->max_combo, s->n300, s->n100,
        s->n50, s->nmiss, s->pp
    );
}

int main(int argc, char* argv[])
{
#ifndef OPPAI_NOCURL
    CURL* curl = 0;
    char url_buf[128];
    char* url = url_buf;
#endif

    char fname_buf[4096];
    char* fname = fname_buf;

    uint32_t i;
    uint32_t n = (uint32_t)(sizeof(suite) / sizeof(suite[0]));

    int32_t err;

    struct parser* pstate = 0;
    struct diff_calc stars;
    struct beatmap map;
    struct beatmap_stats mapstats;
    struct pp_params params;
    struct pp_calc pp;

    fname += sprintf(fname, "test_suite/");

    pstate = (struct parser*)malloc(sizeof(struct parser));
    if (!pstate) {
        check_err(ERR_OOM);
    }

    check_err(p_init(pstate));
    check_err(d_init(&stars));

    for (i = 0; i < n; ++i)
    {
        FILE* f;
        struct score const* s = &suite[i];
        double margin;

        print_score(s);
        sprintf(fname, "%u.osu", s->id);
        f = fopen(fname_buf, "rb");

#ifndef OPPAI_NOCURL
trycalc:
#endif
        err = p_map(pstate, &map, f);
        if (err < 0)
        {
#ifndef OPPAI_NOCURL
            /* TODO: properly error check&log curl
                     also pull this out into a function maybe */

            CURLcode res;

            fprintf(stderr, "%s\n", errstr(err));
            fclose(f);
            f = 0;
            mkdir("test_suite");

            if (!curl)
            {
                fprintf(stderr, "initializing curl\n");
                curl_global_init(CURL_GLOBAL_ALL);

                curl = curl_easy_init();
                if (!curl) {
                    fprintf(stderr, "curl_easy_init failed\n");
                    exit(1);
                }

                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
                curl_easy_setopt(
                    curl, CURLOPT_WRITEFUNCTION, wrchunk
                );

                url += sprintf(url, "http://osu.ppy.sh/osu/");
            }

            sprintf(url, "%u", s->id);
            curl_easy_setopt(curl, CURLOPT_URL, url_buf);

            fprintf(stderr, "downloading %s\n", url_buf);
            f = fopen(fname_buf, "wb");
            if (!f) {
                perror("fopen");
                exit(1);
            }

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform failed");
                fclose(f);
                unlink(fname_buf);
                goto trycalc;
            }

            fclose(f);
            f = 0;
            goto trycalc;
#else
            fprintf(stderr, "please download the test suite from "
                "http://www.hnng.moe/stuff/ and untar here, or "
                "recompile without OPPAI_NOCURL to automatically "
                "download it\n");
#endif /* OPPAI_NOCURL */
        }

        fclose(f);

        check_err(d_calc(&stars, &map, s->mods));

        mapstats.ar = map.ar;
        mapstats.cs = map.cs;
        mapstats.od = map.od;
        mapstats.hp = map.hp;

        mods_apply(s->mods, &mapstats, APPLY_ALL);

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

        if (fabs(pp.total - s->pp) >= margin)
        {
#ifdef OPPAI_DEBUG
            int32_t i;
#endif

            fprintf(
                stderr,
                "failed test: got %g pp, expected %g\n",
                pp.total, s->pp
            );

#ifdef OPPAI_DEBUG
            for (i = 0; i < map.nobjects; ++i)
            {
                struct object* o = &map.objects[i];

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

    return 0;
}

