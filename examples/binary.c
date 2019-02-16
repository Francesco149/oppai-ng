/*
 * example of parsing oppai's binary output
 *
 * gcc binary.c
 * oppai /path/to/file.osu -obinary | ./a.out
 */

#include <stdio.h>
#include <string.h>

/*
 * these are only necessary to ensure endian-ness, if you don't
 * care about that you can read values like v = *(int*)p
 */

int read2(char** c) {
  unsigned char* p = (unsigned char*)*c;
  *c += 2;
  return p[0] | (p[1] << 8);
}

int read4(char** c) {
  unsigned char* p = (unsigned char*)*c;
  *c += 4;
  return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

float read_flt(char** p) {
  int v = read4(p);
  float* pf = (float*)&v;
  return *pf;
}

char* read_str(char** p, int* len) {
  char* res;
  *len = read2(p);
  res = *p;
  *p += *len + 1;
  return res;
}

#define MODS_NF (1<<0)
#define MODS_EZ (1<<1)
#define MODS_HD (1<<3)
#define MODS_HR (1<<4)
#define MODS_DT (1<<6)
#define MODS_HT (1<<8)
#define MODS_NC (1<<9)
#define MODS_FL (1<<10)
#define MODS_SO (1<<12)

int main() {
  char buf[8192];
  char* p = buf;
  int len;
  int result;
  int mods;

  memset(buf, 0, sizeof(buf));

  /* read stdin in binary mode */
  if (!freopen(0, "rb", stdin)) {
    perror("freopen");
    return 1;
  }

  if (!fread(buf, 1, sizeof(buf), stdin)) {
    perror("fread");
    return 1;
  }

  if (strncmp((char const*)p, "binoppai", 8)) {
    puts("invalid input");
    return 1;
  }
  p += 8;

  printf("oppai %d.%d.%d\n", p[0], p[1], p[2]);
  p += 3;
  puts("");

  /* error code */
  result = read4(&p);
  if (result < 0) {
    printf("error %d\n", result);
    return 1;
  }

  printf("artist: %s\n", read_str(&p, &len));
  printf("artist_unicode: %s\n", read_str(&p, &len));
  printf("title: %s\n", read_str(&p, &len));
  printf("title_unicode: %s\n", read_str(&p, &len));
  printf("version: %s\n", read_str(&p, &len));
  printf("creator: %s\n", read_str(&p, &len));

  mods = read4(&p);
  puts("");
  printf("mods: ");
  if (mods & MODS_NF) printf("NF");
  if (mods & MODS_EZ) printf("EZ");
  if (mods & MODS_HD) printf("HD");
  if (mods & MODS_HR) printf("HR");
  if (mods & MODS_DT) printf("DT");
  if (mods & MODS_HT) printf("HT");
  if (mods & MODS_NC) printf("NC");
  if (mods & MODS_FL) printf("FL");
  if (mods & MODS_SO) printf("SO");
  puts("");

  printf("OD%g ", read_flt(&p));
  printf("AR%g ", read_flt(&p));
  printf("CS%g ", read_flt(&p));
  printf("HP%g\n", read_flt(&p));
  printf("%d/%dx\n", read4(&p), read4(&p));
  printf("%d circles ", read2(&p));
  printf("%d sliders ", read2(&p));
  printf("%d spinners\n", read2(&p));
  printf("scorev%d\n", read4(&p));
  puts("");
  printf("%g stars ", read_flt(&p));
  printf("(%g speed, ", read_flt(&p));
  printf("%g aim)\n", read_flt(&p));
  read2(&p); /* legacy */
  read2(&p); /* legacy */
  puts("");
  printf("%g aim pp\n", read_flt(&p));
  printf("%g speed pp\n", read_flt(&p));
  printf("%g acc pp\n", read_flt(&p));
  puts("");
  printf("%g pp\n", read_flt(&p));

  return 0;
}
