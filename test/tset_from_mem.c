/* gcc min.c
   cat /path/to/file.osu | ./a.out */

#define OPPAI_IMPLEMENTATION
#include "../oppai.c"

int main()
{
    struct parser pstate;
    struct beatmap map;

    uint32_t mods;
    struct diff_calc stars;
    struct pp_calc pp;

    uint8_t buf[256*1024];

    FILE *fp=fopen("E:\\osu!\\Songs\\141592 ave;new - Songs Compilation\\ave;new - Songs Compilation (Laurier) [Extra].osu","r");
    size_t nread=fread(buf,1,256*1024,fp);
    fclose(fp);

    p_init(&pstate);
    p_map_mem(&pstate, &map, buf,nread);

    //mods = MODS_HD | MODS_DT;
    mods = 0;
    d_init(&stars);
    d_calc(&stars, &map, mods);
    printf("%g stars\n", stars.total);

    b_ppv2(&map, &pp, stars.aim, stars.speed, mods);
    printf("%gpp\n", pp.total);

    return 0;
}

