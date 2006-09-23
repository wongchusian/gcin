#include "gcin.h"
#include "pho.h"

#define MAX_CHS (15000)

typedef struct {
  u_short key;
  u_char ch[CH_SZ];
  u_short count;
} PHITEM;

PHITEM items[MAX_CHS];
int itemsN;

PHO_ITEM pho_items[MAX_CHS];
int pho_itemsN=0;

static int shiftb[]={9,7,3,0};

int lookup(u_char *s)
{
  int i;
  char tt[CH_SZ+1], *pp;

  if (*s < 128)
    return *s-'0';

  bchcpy(tt, s);
  tt[CH_SZ]=0;

  for(i=0;i<3;i++)
    if (pp=strstr(pho_chars[i],tt))
      break;

  if (i==3)
    return 0;

  return (((pp-pho_chars[i])/CH_SZ) << shiftb[i]);
}


void p_err(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  fprintf(stderr,"gcin:");
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");
  exit(-1);
}

void dbg(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  fflush(stdout);
  va_end(args);
}


int qcmp_key(const void *aa, const void *bb)
{
  PHITEM *a=(PHITEM *)aa;
  PHITEM *b=(PHITEM *)bb;

  int d;
  if (d=a->key - b->key)
    return a->key - b->key;

  return b->count - a->count;
}


main(int argc, char **argv)
{
  char *fname = "pho.tab.src";
  FILE *fp;
  char s[64];
  gboolean reload = getenv("GCIN_NO_RELOAD")==NULL;

  if (reload)
    gtk_init(&argc, &argv);

  if ((fp=fopen(fname,"r"))==NULL)
    p_err("cannot open %s\n", fname);


  while (!feof(fp)) {
    fgets(s,sizeof(s),fp);
    int len=strlen(s);

    if (s[len-1]=='\n')
      s[--len]=0;

    if (len==0)
      continue;

    int i;
    u_short kk=0;
    u_char *p = s;

    while (*p && *p!=' ') {
      kk |= lookup(p);

      if (*p & 128)
        p+=CH_SZ;
      else
        p++;
    }

    items[itemsN].key = kk;

    p++;

    bchcpy(items[itemsN].ch, p);

    p+= CH_SZ + 1; // skip ch & space

    items[itemsN].count = atoi(p);

    itemsN++;
  }

  fclose(fp);

  qsort(items, itemsN, sizeof(PHITEM), qcmp_key);

  int i;

  PHO_IDX pho_idx[3000];
  int pho_idxN=0;

  for(i=0; i < itemsN; ) {
    u_short key = items[i].key;
    pho_idx[pho_idxN].key = key;
    pho_idx[pho_idxN].start = i;
    pho_idxN++;

    int j;

    for (j=i+1; j < itemsN && items[j].key == key; j++);

    int l;
    for(l=i; l<j; l++) {
      bchcpy(pho_items[pho_itemsN].ch, items[l].ch);
#if 0
      char tt[4];
      utf8cpy(tt, items[l].ch);
      dbg("uuu %s\n", tt);
#endif
      pho_items[pho_itemsN].count = items[l].count;
      pho_itemsN++;
    }

    i = j;
  }

  dbg("pho_itemsN:%d  pho_idxN:%d\n", pho_itemsN, pho_idxN);

  char *fname_out = "pho.tab";

  if ((fp=fopen(fname_out,"w"))==NULL)
    p_err("cannot create %s\n", fname_out);

  fwrite("PH",1,2,fp);
  fwrite(&pho_idxN, sizeof(u_short), 1, fp);
  fwrite(pho_idx, sizeof(PHO_IDX), pho_idxN, fp);
  fwrite(pho_items, sizeof(PHO_ITEM), pho_itemsN, fp);

  fclose(fp);

  if (reload)
    send_gcin_message(GDK_DISPLAY(), "reload");

  return 0;
}
