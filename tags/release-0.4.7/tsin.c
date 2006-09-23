/*
	Copyright (C) 2004	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"
#include "tsin.h"


extern PHO_ITEM *ch_pho;
extern char *pho_chars[];

extern int ityp3_pho;
extern u_char typ_pho[];
extern char inph[];

extern u_short idxnum_pho;
extern PHO_IDX idx_pho[];
extern u_short hash_pho[];
extern PHOKBM phkbm;

extern char tsidxfname[64];
extern int hashidx[TSIN_HASH_N];

#define IN_AREA_LEN (8)
#define IN_AREA_LEN_SPC (IN_AREA_LEN + 2)

static u_char ch_buf[MAX_PH_BF_EXT][CH_SZ];
static u_char ch_obuf[MAX_PH_BF_EXT][CH_SZ];
static phokey_t ph_buf[MAX_PH_BF_EXT];
static u_char psta[MAX_PH_BF_EXT];
static int c_idx, c_len, ph_sta=-1, ph_sta_last=-1;
static int sel_pho;
static int eng_ph=1;
static int save_frm, save_to;
static int save_mode;
static int current_page;
static int startf;
static int full_match;
static gboolean tsin_half_full;

static struct {
  phokey_t phokey[MAX_PHRASE_LEN];
  int phidx;
  char str[MAX_PHRASE_LEN*CH_SZ+1];
  int len;
} pre_sel[10];
int pre_selN;

static gboolean use_caps_lock=1;
static gboolean phrase_pre_select=1;

void load_tsin_conf()
{
  char swkey[16];
  get_gcin_conf_str("tsin-chinese-english-switch", swkey, "CapsLock");
  if (!strcmp(swkey, "Tab"))
    use_caps_lock = 0;
  else
    use_caps_lock = 1;

  char phr_pre[16];
  get_gcin_conf_str("tsin-phrase-pre-select", phr_pre, "1");
  phrase_pre_select = atoi(phr_pre);
}


static void disp_char_chbuf(int idx)
{
  disp_char(idx, ch_buf[idx]);
}


static void clrcursor()
{
  clr_tsin_cursor(c_idx);
}

static int last_cursor_idx=0;

static void drawcursor()
{
  clr_tsin_cursor(last_cursor_idx);
  last_cursor_idx = c_idx;

  if (c_idx == c_len) {
    if (tsin_half_full || eng_ph) {
      disp_char(c_idx,"　");
      set_cursor_tsin(c_idx);
    } else {
      disp_char(c_idx, "  ");
      set_cursor_tsin(c_idx);
    }
  }
  else {
    set_cursor_tsin(c_idx);
  }

  gdk_flush();
}


static void putbuf(u_char s[][CH_SZ], int len)
{
  u_char tt[CH_SZ * (MAX_PH_BF_EXT+1) + 1];
  int i,idx;

#if 0
  // update phrase reference count
  if (len >= 2) {
    for(idx = len -1; i >= 1; ) {
      int tlen = 0;

      for(tlen=MIN(len, MAX_PHRASE_LEN); tlen>1; tlen++) {
      }
    }
  }
#endif

  for(idx=i=0;i<len;i++) {
    if (s[i][0]>=128) {
      int pho_idx = ch_key_to_ch_pho_idx(ph_buf[i], &s[i][0]);

      if (pho_idx >= 0)
        inc_pho_count(ph_buf[i], pho_idx);

      bchcpy(&tt[idx],&s[i][0]);
      idx+=CH_SZ;
    }
    else
      tt[idx++]=s[i][0];
  }

  tt[idx]=0;
  send_text(tt);
}


static void prbuf()
{
  int i;

//  dbg("prbuf\n");

  for(i=0; i < c_len; i++) {
    disp_char_chbuf(i);
  }

  if (c_len < MAX_PH_BF_EXT)
    hide_char(c_len);

  if (c_len+1 < MAX_PH_BF_EXT)
    hide_char(c_len+1);

  drawcursor();
}


static int orig_disp_in_area_x = -1;

static void clr_in_area_pho_tsin()
{
  int i;

  for(i=0; i < 4; i++)
   disp_tsin_pho(i, "　");
}

static void disp_in_area_pho_tsin()
{
  int i;

  for(i=0;i<4;i++) {
    disp_tsin_pho(i, &pho_chars[i][typ_pho[i] * CH_SZ]);
  }
}


static void restore_ai()
{
  if (sel_pho)
    return;

  clear_chars_all();
  clrin_pho();
  disp_in_area_pho_tsin();
  prbuf();
}

static void clear_disp_ph_sta();
static void clear_match()
{
  ph_sta=-1;
  clear_disp_ph_sta();
  bzero(psta, sizeof(psta));
//  dbg("clear_match\n");
}

static void clr_ch_buf()
{
  int i;
  for(i=0; i < MAX_PH_BF_EXT; i++) {
    ch_buf[i][0]=' ';
    ch_buf[i][1]=0;
    psta[i]=0xff;
  }

  clear_match();
}


void show_stat()
{
  disp_tsin_eng_pho(eng_ph);
}

static void load_tsin_entry(int idx, u_char *len, char *usecount, phokey_t *pho,
                    u_char *ch)
{
  if (idx >= phcount) {
    load_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  int ph_ofs=phidx[idx];

  fseek(fph, ph_ofs, SEEK_SET);
  fread(len, 1, 1, fph);

  if (*len >= MAX_PHRASE_LEN) {
    load_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  fread(usecount, 1, 1,fph); // use count
  fread(pho, sizeof(phokey_t), (int)(*len), fph);
  if (ch)
    fread(ch, CH_SZ, (int)(*len), fph);
}


#if 0
void nputs(u_char *s, u_char len)
{
  char tt[16];

  memcpy(tt, s, len*CH_SZ);
  tt[len*CH_SZ]=0;
  dbg("%s", tt);
}


static void dump_tsidx(int i)
{
  phokey_t pho[MAX_PHRASE_LEN];
  u_char ch[MAX_PHRASE_LEN*CH_SZ];
  char usecount;
  u_char len;

  load_tsin_entry(i, &len, &usecount, pho, ch);

  int j;
  for(j=0; j < len; j++) {
    prph(pho[j]);
    dbg(" ");
  }

  nputs(ch, len);
  dbg("\n");
}


static void dump_tsidx_all()
{
  int i;

  for(i=0; i < phcount; i++) {
    dump_tsidx(i);
  }

  dbg("************************************************\n");
  for(i=0; i < 254; i++) {
    dbg("%d]%d ", i, hashidx[i]);
    dump_tsidx(hashidx[i]);
  }
}

#endif


void load_tab_pho_file();

void init_tab_pp(int usenow)
{
  FILE *fr;
  int i,cou;
  unsigned int ttt;
  extern char *tabfname[];
  char phofname[128];

  if (!ch_pho)
    load_tab_pho_file();

  if (phcount && gwin0) {
disp_prom:
    show_stat();
    restore_ai();
    show_win0();
    return;
  }

  load_tsin_db();
  load_tsin_conf();

  clr_ch_buf();
  show_win0();

//  dump_tsidx_all();
  goto disp_prom;
}


static void save_phrase()
{
  int tt, ofs, top,bottom, mid, ord, ph_ofs, hashno, hashno_end, i;
  FILE *fw;
  u_char len, tbuf[24], sbuf[24], ch[16];

  if (save_frm==save_to)
    return;

  if (save_frm > save_to) {
    tt=save_frm;
    save_frm=save_to;
    save_to=tt;
  }

  if (save_to==c_len)
    save_to--;

  len= save_to- save_frm+ 1;
  if (len < 2 || len > MAX_PHRASE_LEN)
    return;

  for(i=save_frm;i<=save_to;i++)
    if (!ph_buf[i])
      return;

  if (!save_phrase_to_db(&ph_buf[save_frm], &ch_buf[save_frm], len)) {
    bell();
  }

cursor_end:
  clrcursor();
  ph_sta=-1;
  c_idx=c_len;
  drawcursor();
  return;
}

#define PH_SHIFT_N (MAX_PH_BF - 1)

static void shift_ins()
{
   int i,j;

   if (!c_idx && c_len >= PH_SHIFT_N) {
     c_len--;
   }
   else
   if (c_len >= PH_SHIFT_N) {
     int ofs;

     if (!(ch_buf[0][0] & 0x80) && (ch_buf[1][0]&0x80)) {
       putbuf(ch_buf,2);
       ofs=2;
     }
     else
     if (!(ch_buf[0][0]&0x80) && !(ch_buf[1][0]&0x80)) {
       putbuf(ch_buf,2);
       ofs=2;
     } else {
       putbuf(ch_buf,1);
       ofs=1;
     }
     ph_sta-=ofs;
     for(j=0;j<PH_SHIFT_N-ofs;j++) {
       bchcpy(ch_buf[j], ch_buf[j+ofs]);
       bchcpy(ch_obuf[j], ch_obuf[j+ofs]);
       ph_buf[j]=ph_buf[j+ofs];
       psta[j]=psta[j+ofs]-ofs;
     }
     c_idx-=ofs;
     c_len-=ofs;
     prbuf();
   }

   c_len++;
   if (c_idx < c_len-1) {
     for(j=c_len; j>=c_idx; j--) {
       bchcpy(ch_buf[j+1], ch_buf[j]);
       bchcpy(ch_obuf[j+1], ch_obuf[j]);
       ph_buf[j+1]=ph_buf[j];
       psta[j+1]=psta[j]+1;
     }
      ch_buf[c_len][0]=' ';
    /*    prbuf(); */
   }

   compact_win0();
}


void put_b5_char(char *b5ch, phokey_t key)
{
   shift_ins();

   bchcpy(ch_buf[c_idx], b5ch);
   bchcpy(ch_obuf[c_idx], b5ch);

   disp_char_chbuf(c_idx);

   ph_buf[c_idx]=key;
   c_idx++;

   if (c_idx < c_len) {
     prbuf();
   }
}


#define MAX_PHRASE_SEL_N (9)

static u_char selstr[MAX_PHRASE_SEL_N][MAX_PHRASE_LEN * CH_SZ], sellen[MAX_PHRASE_SEL_N];
static int selidx[MAX_PHRASE_SEL_N];

static u_short phrase_count;
static u_short pho_count;

static void get_sel_phrase()
{
  int sti,edi,i,j;
  phokey_t key, stk[10];
  u_char len, mlen, stch[MAX_PHRASE_LEN * CH_SZ + 1];
  int cnd=0;

  mlen=c_len-c_idx;

  if (mlen > MAX_PHRASE_LEN)
    mlen=MAX_PHRASE_LEN;

  key=ph_buf[c_idx];
  j= key >> TSIN_HASH_SHIFT;

  if (j >= TSIN_HASH_N)
    return;

  sti=hashidx[j];
  edi=hashidx[j+1];
  phrase_count = 0;

  while (sti < edi && phrase_count < MAX_PHRASE_SEL_N) {
    char usecount=0;
    load_tsin_entry(sti, &len, &usecount, stk, stch);

    if (len > mlen) {
      sti++;
      continue;
    }

    if (!memcmp(&ph_buf[c_idx], stk, sizeof(phokey_t)*len)) {
      sellen[phrase_count]=len;
      selidx[phrase_count]=sti;
      memcpy(selstr[phrase_count++], stch, CH_SZ*len);
    }

    sti++;
  }
}



static void get_sel_pho()
{
  phokey_t key;

  if (c_idx==c_len)
    key=ph_buf[c_idx-1];
  else
    key=ph_buf[c_idx];

  if (!key)
    return;

  int i=hash_pho[key>>9];
  phokey_t ttt;

  while (i<idxnum_pho) {
    ttt=idx_pho[i].key;
    if (ttt>=key)
      break;
    i++;
  }

  if (ttt!=key) {
    return;
  }

  startf = idx_pho[i].start;
  pho_count = idx_pho[i+1].start - startf;
//  dbg("pho_count %d\n", pho_count);
}


static int disp_current_sel_page()
{
  int i;

  clear_sele();

  for(i=0; i < SELKEY; i++) {
    int idx = current_page + i;

    if (idx < phrase_count) {
      set_sele_text(i, selstr[i], CH_SZ*sellen[i]);
    } else
    if (idx < phrase_count + pho_count) {
      int v = idx - phrase_count + startf;
      set_sele_text(i, ch_pho[v].ch, CH_SZ);
    } else
      break;
  }

  if (current_page + SELKEY < phrase_count + pho_count) {
    disp_arrow_down();
  }

  if (current_page > 0)
    disp_arrow_up();

  disp_selections(c_idx);
}

static int fetch_user_selection(int val, char **seltext)
{
  int idx = current_page + val;
  int len = 0;

  if (idx < phrase_count) {
    len = CH_SZ*sellen[idx];

    *seltext = selstr[idx];
  } else
  if (idx < phrase_count + pho_count) {
    int v = idx - phrase_count + startf;

    len = CH_SZ;
    *seltext = ch_pho[v].ch;
  }

  return len;
}


static int phokey_t_seq(phokey_t *a, phokey_t *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}



static u_char scanphr(phokey_t *pp, int plen)
{
  int mid, cmp;
  phokey_t ss[MAX_PHRASE_LEN], stk[MAX_PHRASE_LEN];
  u_char len, mlen, usecount, stch[MAX_PHRASE_LEN * CH_SZ];
  int sti,edi;
  int i= *pp>> TSIN_HASH_SHIFT;

  if (i >= TSIN_HASH_N)
    return 0;

  int top=hashidx[i];
  int bot=edi=hashidx[i+1];

  while (top <= bot) {
    mid=(top+bot)/ 2;
    sti++;
    load_tsin_entry(mid, &len, &usecount, ss, stch);

    if (len > plen)
      mlen=plen;
    else
      mlen=len;

    cmp=phokey_t_seq(ss, pp, mlen);

    if (!cmp && len < plen)
      cmp=-2;

    if (cmp>0)
      bot=mid-1;
    else
    if (cmp<0)
      top=mid+1;
    else
      break;
  }

  if (cmp) {
//    dbg("no match %d\n", cmp);
    return 0;
  }

  // seek to the first match because binary search is used
  for(;mid>=0;mid--) {
    load_tsin_entry(mid, &len, &usecount, stk, stch);

    if (len >= plen && !phokey_t_seq(stk, pp, plen))
      continue;
    break;
  }

  mid++;
  sti = mid;

#if 0
  load_tsin_entry(mid, &len, &usecount, stk, stch);
  dbg("scanphr ");
  nputs(stch, len);
  dbg("len:%d\n", len);
#endif

  pre_selN = 0;
  int maxlen=0;

  while (sti < edi && pre_selN < 10) {
    phokey_t mtk[MAX_PHRASE_LEN];
    u_char mtch[MAX_PHRASE_LEN*CH_SZ+1];
    u_char match_len;

    load_tsin_entry(sti, &match_len, &usecount, mtk, mtch);
    sti++;
    if (plen > match_len) {
      continue;
    }

    int i;
    for(i=0;i < plen;i++) {
      if (mtk[i]!=pp[i])
        break;
    }

    if (i < plen)
      continue;

#if 0
    dbg("nnn ");
    nputs(mtch, match_len);
    dbg("\n");
#endif

    if (maxlen < match_len)
      maxlen = match_len;

    pre_sel[pre_selN].len = match_len;
    pre_sel[pre_selN].phidx = sti - 1;
    memcpy(pre_sel[pre_selN].str, mtch, match_len*CH_SZ);
    memcpy(pre_sel[pre_selN].phokey, mtk, match_len*sizeof(phokey_t));
    pre_selN++;
  }

  return maxlen;
}

static void disp_pre_sel_page()
{

  int i;

  if (!phrase_pre_select) {
    return;
  }

  if (pre_selN==0 || pre_selN==1 && pre_sel[0].len==2)
    return;

  clear_sele();

  for(i=0; i < Min(SELKEY, pre_selN); i++) {
    set_sele_text(i, pre_sel[i].str, pre_sel[i].len*CH_SZ);
  }

  disp_selections(ph_sta);
}


static raise_phr(int c)
{
  int i,j,tmp;
  FILE *fp;

  if (!c)
    return;

  i=selidx[c];
  j=selidx[0];
  tmp=phidx[i];
  phidx[i]=phidx[j];
  phidx[j]=tmp;
  selidx[0]=tmp;

  if ((fp=fopen(tsidxfname,"r+"))==NULL) {
    dbg("%s modify err", tsidxfname);
    return;
  }

  fseek(fp,sizeof(phcount)+sizeof(hashidx) + j * sizeof(int), SEEK_SET);
  fwrite(&phidx[j],4,(c+1),fp);
  fclose(fp);
}


static void close_selection_win()
{
  hide_selections_win();
  current_page=sel_pho=0;
  pre_selN = 0;
}


static void clear_ch_buf_sel_area()
{
  clear_chars_all();
  c_len=c_idx=0; ph_sta=-1;
  full_match = FALSE;
  clr_ch_buf();
  drawcursor();
  clear_disp_ph_sta();
}


static void clear_tsin_buffer()
{
  clear_ch_buf_sel_area();
  close_selection_win();
  pre_selN = 0;
}


gboolean flush_tsin_buffer()
{

  if (c_len) {
    putbuf(ch_buf,c_len);
    compact_win0();
    clear_ch_buf_sel_area();
    clear_tsin_buffer();
    return 1;
  }

  return 0;
}


void tsin_set_eng_ch(int nmod)
{
  eng_ph = nmod;
  show_stat();
  drawcursor();
}

void tsin_toggle_eng_ch()
{
  tsin_set_eng_ch(eng_ph ^= 1);
}

void tsin_toggle_half_full()
{
  if (!eng_ph) {
    tsin_half_full^=1;
    drawcursor();
  } else {
    flush_tsin_buffer();
  }
}


static char ochars[]="<,>.?/:;\"'{[}]_-+=|\\~`";

static void hide_pre_sel()
{
  pre_selN = 0;
  hide_selections_win();
}


void draw_line(int att, int x0, int y0, int x1, int y1);

static void clear_disp_ph_sta()
{
  clear_tsin_line();
}

static void disp_ph_sta()
{
  clear_disp_ph_sta();

  if (ph_sta < 0)
    return;

  int i;

  for(i=ph_sta; i < c_idx; i++) {
    draw_underline(i);
  }
}


gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len)
{
    int idx = c_idx;
    int i;

    if (idx < 0 || c_len + len >= MAX_PH_BF)
      return 0;

    for(i=0; i < len; i++) {
      if (pho[i])
        continue;

      phokey_t tpho[32];
      big5_pho_chars(&str[i*CH_SZ], tpho);
      pho[i] = tpho[0];
    }


#if 1
    if (idx < c_len) {
      for(i=c_len-1; i >= idx; i--) {
        memcpy(ch_buf[i+len], ch_buf[i], CH_SZ);
        memcpy(ch_obuf[i+len], ch_obuf[i], CH_SZ);
        ph_buf[i+len] = ph_buf[i];
      }
    }

    memcpy(&ch_buf[idx], str, len*CH_SZ);
    memcpy(&ph_buf[idx], pho, len*sizeof(phokey_t));

    if (c_idx == c_len)
      c_idx +=len;

    c_len+=len;

    compact_win0();
    clrin_pho();
    disp_in_area_pho_tsin();

    prbuf();
    for(i=0;i < len; i++) {
      psta[idx+i]=idx;
    }
#else
    int i;

    for(i=0; i < len; i++) {
      put_b5_char(&str[i*CH_SZ], pho[i]);
    }

    prbuf();
#endif

    drawcursor();
    disp_ph_sta();
    hide_pre_sel();
    ph_sta=-1;
    return TRUE;
}



gboolean add_to_tsin_buf_phsta(char *str, phokey_t *pho, int len)
{
    int idx = ph_sta < 0 && c_idx==c_len ? ph_sta_last : ph_sta;
#if 0
    dbg("idx:%d  ph_sta:%d ph_sta_last:%d c_idx:%d  c_len:%d\n",
       idx, ph_sta, ph_sta_last, c_idx, c_len);
#endif
    if (idx < 0)
      return 0;

    if (idx >= MAX_PH_BF)
      flush_tsin_buffer();

    memcpy(&ch_buf[idx], str, len*CH_SZ);
    memcpy(&ph_buf[idx], pho, len*sizeof(phokey_t));
    c_len=c_idx=idx + len;

    clrin_pho();
    disp_in_area_pho_tsin();

    prbuf();

    int j;
    for(j=0;j < len; j++) {
      psta[idx+j]=idx;
    }

    drawcursor();
    disp_ph_sta();
    hide_pre_sel();
    ph_sta=-1;
    return 1;
}

static gboolean pre_sel_handler(KeySym xkey)
{
  static char shift_sele[]="!@#$%^&*()asdfghjkl:";
  static char noshi_sele[]="1234567890asdfghjkl;";
  char *p;

  if (!pre_selN || !phrase_pre_select)
    return 0;

  if (isupper(xkey))
    xkey = xkey - 'A' + 'a';

//  dbg("pre_sel_handler aa\n");

  if (!(p=strchr(shift_sele, xkey)))
    return 0;

  int c = p - shift_sele;
  char noshi = noshi_sele[c];

  if (!(p=strchr(phkbm.selkey, noshi)))
    return 0;

  c = p - phkbm.selkey;

  int len = pre_sel[c].len;

  if (c >= pre_selN)
    return;

  int j, eqlenN=0, current_ph_idx;

  for(j=0; j < pre_selN; j++) {
    if (pre_sel[j].len != len || phokey_t_seq(pre_sel[j].phokey, pre_sel[c].phokey, len))
      continue;

    if (j==c)
      current_ph_idx = eqlenN;

    selidx[eqlenN++]=pre_sel[j].phidx;
  }

//    dbg("eqlenN:%d %d\n", eqlenN, current_ph_idx);

  if (eqlenN > 1) {
    raise_phr(current_ph_idx);
  }

  full_match = 0;
  return add_to_tsin_buf_phsta(pre_sel[c].str, pre_sel[c].phokey, len);
}


static gboolean pre_punctuation(KeySym xkey)
{
  static char shift_punc[]="<>?:\"{}";
  static char chars[]="，。？：；『』";

  char *p;

  if (p=strchr(shift_punc, xkey)) {
    int c = p - shift_punc;
    phokey_t key=0;

    return add_to_tsin_buf(&chars[c*CH_SZ], &key, 1);
  }

  return 0;
}


gboolean feedkey_pp(KeySym xkey, int kbstate)
{
  static char ctyp;
  static u_int ii;
  static u_short key;
  int i,k,pst;
  u_char match_len;
  char usecount;
  extern u_char fullchar[];
  int shift_m=kbstate&ShiftMask;
  int j,jj,kk, idx;
  char kno;

   if (kbstate & (Mod1Mask|Mod1Mask)) {
       return 0;
   }

   switch (xkey) {
     case XK_Escape:
          save_mode=0;
          clrin_pho();
          prbuf();
          clr_in_area_pho_tsin();
          close_selection_win();
          pre_selN = 0;
          drawcursor();
          return 1;
     case XK_Return:
     case XK_KP_Enter:
        if (shift_m) {
          save_frm=c_idx;
          save_to=c_len-1;
          save_phrase();
          return 1;
        } else
          return flush_tsin_buffer();
     case XK_Home:
        close_selection_win();
        if (!c_len)
          return 0;
        clrcursor();
        c_idx=0;
        drawcursor();
        return 1;
     case XK_End:
        close_selection_win();
        if (!c_len)
          return 0;
        clrcursor();
        c_idx=c_len;
        drawcursor();
        return 1;
     case XK_Left:
        close_selection_win();
        if (c_idx) {
          clrcursor();
          c_idx--;
          drawcursor();
          return 1;
        }
        return 0;
     case XK_Right:
        close_selection_win();
        if (c_idx < c_len) {
          clrcursor();
          c_idx++;
          drawcursor();
          return 1;
        }
        return 0;
     case XK_Caps_Lock:
        if (use_caps_lock) {
          close_selection_win();
          tsin_toggle_eng_ch();
          return 1;
        } else
          return 0;
     case XK_Tab:
        if (!use_caps_lock) {
          close_selection_win();
          tsin_toggle_eng_ch();
          return 1;
        } else {
          if (c_len) {
            flush_tsin_buffer();
            return 1;
          }
          return 0;
        }
#if 0
     case XK_KP_Multiply:
        close_selection_win();
        if (save_mode) {
          save_to=c_idx;
          save_phrase();
          save_mode=0;
          bell();
        } else {
          save_frm=c_idx;
          save_mode=1;
          bell();
        }
        return 1;
#endif
     case XK_Delete:
        if (c_idx == c_len)
          return 0;

        close_selection_win();
        clear_disp_ph_sta();
        ityp3_pho=0;
        pre_selN = 0;

        for(j=3;j>=0;j--)
          if (typ_pho[j]) {
            typ_pho[j]=0;
            disp_in_area_pho_tsin();
            return 1;
          }

        clrcursor();
        pst=k=psta[c_idx];

        for(k=c_idx;k<c_len;k++) {
          memcpy(ch_buf[k], ch_buf[k+1], CH_SZ);
          memcpy(ch_obuf[k], ch_obuf[k+1], CH_SZ);
          ph_buf[k]=ph_buf[k+1];
          psta[k]=psta[k+1]-1;
        }

        c_len--;
        prbuf();
        compact_win0();

        if (!c_idx) {
          clear_match();
        }
        else {
          k=c_idx-1;
          pst=psta[k];

          while (k>0 && psta[k]==pst)
            k--;

          if (psta[k]!=pst)
            k++;

          match_len= c_idx - k;
          if (!(match_len=scanphr(&ph_buf[k], match_len)))
            ph_sta=-1;
          else
            ph_sta=k;

//          if (ph_sta < 0 || c_idx - ph_sta < 2)
            pre_selN = 0;
        }

        disp_ph_sta();
        return 1;
     case XK_BackSpace:
        close_selection_win();
        clear_disp_ph_sta();
        ityp3_pho=0;
        pre_selN = 0;

        for(j=3;j>=0;j--)
          if (typ_pho[j]) {
            typ_pho[j]=0;
            disp_in_area_pho_tsin();
            return 1;
          }

        if (!c_idx)
          return 0;

        clrcursor();
        c_idx--;
        pst=k=psta[c_idx];

        for(k=c_idx;k<c_len;k++) {
          memcpy(ch_buf[k], ch_buf[k+1], CH_SZ);
          memcpy(ch_obuf[k], ch_obuf[k+1], CH_SZ);
          ph_buf[k]=ph_buf[k+1];
          psta[k]=psta[k+1]-1;
        }

        c_len--;
        prbuf();
        compact_win0();

        if (!c_idx) {
          clear_match();
        }
        else {
          k=c_idx-1;
          pst=psta[k];

          while (k>0 && psta[k]==pst)
            k--;

          if (psta[k]!=pst)
            k++;

          match_len= c_idx - k;
          if (!(match_len=scanphr(&ph_buf[k], match_len)))
            ph_sta=-1;
          else
            ph_sta=k;

//          if (ph_sta < 0 || c_idx - ph_sta < 2)
            pre_selN = 0;
        }

        disp_ph_sta();
        return 1;
     case XK_Up:
       if (!sel_pho)
         return 0;

       current_page = current_page - SELKEY;
       if (current_page < 0)
         current_page = 0;

       disp_current_sel_page();
       return 1;
     case XK_space:
     case XK_Down:
       if (!eng_ph && xkey == XK_space)
           goto asc_char;

       if (!ityp3_pho && (typ_pho[0]||typ_pho[1]||typ_pho[2]) && xkey==XK_space) {
         ctyp=3;
         kno=0;

         if (typ_pho[0] && !typ_pho[1] && !typ_pho[2] && !typ_pho[3]) {
           char tc=inph[0];
           if(phkbm.phokbm[tc][1].typ==2) {
             typ_pho[0]=0;
             typ_pho[2]=phkbm.phokbm[tc][1].num;
           }
         }
         goto llll1;
       }

       if (!c_len)
         return 0;

       idx = c_idx==c_len ? c_idx - 1 : c_idx;
       if (!ph_buf[idx])
         return 1;

       if (!sel_pho) {
         get_sel_phrase();
         get_sel_pho();
         sel_pho=1;
         current_page = 0;
       } else {
         current_page = current_page + SELKEY;
         if (current_page >= phrase_count + pho_count)
           current_page = 0;
       }

       disp_current_sel_page();
       return 1;
     default:
       if ((kbstate & ControlMask)) {
         if (xkey==XK_u) {
           clear_tsin_buffer();
           return 1;
         } else
           return 0;
       }


       char *pp;

       if ((pp=strchr(phkbm.selkey,xkey)) && sel_pho) {
         int c=pp-phkbm.selkey;
         char *sel_text;
         int len = fetch_user_selection(c, &sel_text);

         if (len > CH_SZ) {
           memcpy(ch_buf[c_idx], sel_text, len);
           memcpy(ch_obuf[c_idx], sel_text, len);
           raise_phr(c);
         } else
         if (len == CH_SZ) { // single chinese char
           i= c_idx==c_len?c_idx-1:c_idx;
           key=ph_buf[i];
           memcpy(ch_buf[i], sel_text, CH_SZ);
#if 1
           memcpy(ch_obuf[i], sel_text, CH_SZ);
#endif
           if (i && psta[i]<i)
             memcpy(ch_buf[i-1],ch_obuf[i-1], CH_SZ);
         }

         if (len) {
           prbuf();
           current_page=sel_pho=ityp3_pho=0;
           ph_sta=-1;
           hide_selections_win();
         }
         return 1;
       }

       sel_pho=current_page=0;
   }

   if (!eng_ph || shift_m || (xkey <= XK_KP_9 && xkey >= XK_KP_0) ||
        xkey == XK_KP_Subtract || xkey == XK_KP_Add || xkey == XK_KP_Multiply ||
        xkey == XK_KP_Divide
       ) {
asc_char:
        if (shift_m) {
          if (pre_sel_handler(xkey)) {
            return 1;
          }

          if (eng_ph && pre_punctuation(xkey))
            return 1;
        }

        if (xkey <= XK_KP_9 && xkey >= XK_KP_0)
          xkey=xkey-XK_KP_0+'0';
        else {
          switch (xkey) {
            case XK_KP_Add:
              xkey = '+';
              break;
            case XK_KP_Subtract:
              xkey = '-';
              break;
            case XK_KP_Multiply:
              xkey = '*';
              break;
            case XK_KP_Divide:
              xkey = '/';
              break;
          }
        }

//        dbg("xkey: %c\n", xkey);

        if (shift_m && eng_ph)  {
          char *ppp=strchr(ochars,xkey);

          if (!(kbstate&LockMask) && ppp && !((ppp-ochars) & 1))
            xkey=*(ppp+1);
          if (kbstate&LockMask && islower(xkey))
            xkey-=0x20;
          else
            if (!(kbstate&LockMask) && isupper(xkey))
              xkey+=0x20;
        } else {
          if (!eng_ph && use_caps_lock) {
            if (shift_m) {
              if (islower(xkey))
                xkey-=0x20;
            } else
            if (isupper(xkey))
              xkey+=0x20;
          }
        }


        if (xkey > 127)
          return 1;

        u_char tt=xkey;
        shift_ins();

        if (tsin_half_full) {
          bchcpy(ch_buf[c_idx], half_char_to_full_char(xkey));
        } else {
//          dbg("%c\n", tt);
          ch_buf[c_idx][0]=tt;
        }

        phokey_t tphokeys[32];
        tphokeys[0]=0;
        big5_pho_chars(ch_buf[c_idx], tphokeys);

        disp_char_chbuf(c_idx);
        ph_buf[c_idx]=tphokeys[0];
        c_idx++;
        if (c_idx < c_len)
          prbuf();

        orig_disp_in_area_x = -1;

        drawcursor();
        return 1;
   } else { /* pho */
        if (xkey > 127)
          return 0;

        if (xkey >= 'A' && xkey <='Z')
          xkey+=0x20;

        ctyp=-1;
        for(i=2;i>=0;i--)
          if (typ_pho[i])
            break;

        kno=phkbm.phokbm[xkey][0].num;
        ctyp=phkbm.phokbm[xkey][0].typ;

        for(j=0;j<3;j++)
          if (phkbm.phokbm[xkey][j].typ > i) {
            kno=phkbm.phokbm[xkey][j].num;
            ctyp=phkbm.phokbm[xkey][j].typ;
            break;
          }

        if (!kno)
          return 0;

        typ_pho[ctyp]=kno;
        inph[ctyp]=xkey;
llll1:
        jj=0;
        kk=1;
llll2:
        if (ctyp==3)
          ityp3_pho=1;  /* last key is entered */

        disp_in_area_pho_tsin();

        key = pho2key(typ_pho);

        int vv=hash_pho[typ_pho[0]];
        phokey_t ttt=0xffff;
        while (vv<idxnum_pho) {
          ttt=idx_pho[vv].key;
          if (!typ_pho[0]) ttt &= ~(31<<9);
          if (!typ_pho[1]) ttt &= ~(3<<7);
          if (!typ_pho[2]) ttt &= ~(15<<3);
          if (!typ_pho[3]) ttt &= ~(7);
          if (ttt>=key) break;
          else
          vv++;
        }

        if (ttt > key || (ityp3_pho && idx_pho[vv].key!=key) ) {
          while (jj<4) {
            while(kk<3)
            if (phkbm.phokbm[inph[jj]][kk].num ) {
              if (kk) {
                ctyp=phkbm.phokbm[inph[jj]][kk-1].typ;
                typ_pho[ctyp]=0;
              }
              kno=phkbm.phokbm[inph[jj]][kk].num;
              ctyp=phkbm.phokbm[inph[jj]][kk].typ;
              typ_pho[ctyp]=kno;
              kk++;
              goto llll2;
            } else kk++;
            jj++;
            kk=1;
          }

          bell(); ityp3_pho=typ_pho[3]=0;
          disp_in_area_pho_tsin();
          return 1;
        }

        if (key==0 || !ityp3_pho)
          return 1;

        ii=idx_pho[vv].start;
        start_idx=ii;
   } /* pho */

   put_b5_char(ch_pho[start_idx].ch, key);

   disp_ph_sta();
   clrin_pho();
   clr_in_area_pho_tsin();
   drawcursor();
   hide_pre_sel();

   if (ph_sta < 0) {
restart:
     if (match_len=scanphr(&ph_buf[c_idx-1],1))
       ph_sta=c_idx-1;

//     dbg("ph_sta < 0, scanphr c_idx:%d match_len:%d\n", c_idx, match_len);
     psta[c_idx-1]=c_idx-1;
#if 0
     disp_pre_sel_page();
#else
     pre_selN=0;
#endif
     disp_ph_sta();

     return 1;
   } else {
     int mdist = c_idx - ph_sta;
     int max_match_phrase_len;

//     dbg("match_len:%d mdist %d = c_idx:%d - ph_sta:%d\n", match_len,  mdist, c_idx, ph_sta);

     while (ph_sta < c_idx) {
//       dbg("ph_sta:%d\n", ph_sta);
       if (max_match_phrase_len = scanphr(&ph_buf[ph_sta], c_idx - ph_sta)) {
//         dbg("max_match_phrase_len: %d\n", max_match_phrase_len);
         break;
       } else
       if (full_match) {  // tstr: 選擇視窗
//         dbg("last full_match\n");
         full_match = 0; ph_sta = -1;
         goto restart;
       }

       ph_sta++;
     }

     mdist = c_idx - ph_sta;

     if (ph_sta==c_idx) {
//       dbg("uuu no match ....\n");
       clear_match();
       return 1;
     }

     if (!pre_selN) {
//       dbg("no match found ..\n");
       clear_match();
       goto restart;
     }

//     dbg("iiiiiiii ph_sta:%d %d  max_match_phrase_len:%d\n", ph_sta, c_idx, max_match_phrase_len);
     disp_ph_sta();

     if (mdist == 1) {
//       dbg("single len match\n");
       psta[c_idx-1]=c_idx-1;
       pre_selN=0;
       return 1;
     }

     disp_pre_sel_page();

     full_match = FALSE;

     for(i=0; i < pre_selN; i++) {
       if (pre_sel[i].len != mdist)
         continue;

       memcpy(&ch_buf[ph_sta], pre_sel[i].str, mdist*CH_SZ);
       memcpy(&ph_buf[ph_sta], pre_sel[i].phokey, mdist*sizeof(phokey_t));

       int j;
       for(j=0;j < mdist; j++) {
         psta[ph_sta+j]=ph_sta;
         disp_char(ph_sta+j, &pre_sel[i].str[j * CH_SZ]);
       }

       full_match = TRUE;

       if (mdist==max_match_phrase_len) { // tstr: 選擇視窗
//         dbg("full match .......... %d\n", ph_sta);
         ph_sta_last = ph_sta;
         ph_sta = -1;
         if (pre_selN == 1)
           pre_selN = 0;
       }

       return 1;
     }
   }

   return 1;
}
