#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <string.h>
#include "IMdkit.h"
#include "Xi18n.h"
#include "IC.h"


#define CH_SZ (3) // big5 size ( 2 bytes), will be changed to 3 if unicode is used

#define tmalloc(type,n)  (type*)malloc(sizeof(type) * (n))
#define trealloc(p,type,n)  (type*)realloc(p, sizeof(type) * (n+1))

extern Display *dpy;
void p_err(char *fmt,...);
void dbg(char *fmt,...);

extern GtkWidget *gwin0;
extern GdkWindow *gdkwin0;
extern Window xwin0;
extern Window root;
void loadIC();
IC *FindIC(CARD16 icid);
extern IC *current_IC;

#define MROW 2
#define MCOL 39

typedef enum {
  InputStyleOverSpot = 1,
  InputStyleRoot = 2,
  InputStyleOnSpot = 4
} InputStyle_E;


typedef enum {
  Control_Space=0,
  Shift_Space=1,
  Alt_Space=2,
  Windows_Space=3,
} IM_TOGGLE_KEYS;

#define ROW_ROW_SPACING (2)


#define MAX_GCIN_STR (256)

#define PHO_KBM "phokbm"

extern int win_xl, win_yl;
extern int win_x, win_y;
extern int dpy_xl, dpy_yl;

extern int gcin_font_size;

void big5_utf8(char *s, char out[]);
void utf8_big5(char *s, char out[]);
gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

#include "gcin-conf.h"

#define MAX_TAB_KEY_NUM 5

#define bchcpy(a,b) memcpy(a,b, CH_SZ)
#define bchcmp(a,b) memcmp(a,b, CH_SZ)

int utf8_sz(char *s);
void utf8cpy(char *t, char *s);

void get_gcin_dir(char *tt);
Atom get_gcin_atom(Display *dpy);
void get_sys_table_file_name(char *name, char *fname);
char *half_char_to_full_char(KeySym xkey);
void send_text(char *text);
void sendkey_b5(char *bchar);
void bell();