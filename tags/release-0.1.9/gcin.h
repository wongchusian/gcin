#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "IMdkit.h"
#include "Xi18n.h"
#include "IC.h"

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
  InputStyleRoot = 2
} InputStyle_E;


typedef enum {
  NormalColor=0,
  InAreaColor=1,
  CursorColor=2
} TEXT_COLOR;

#define ROW_ROW_SPACING (2)

void get_gcin_dir(char *tt);
Atom get_gcin_atom(Display *dpy);

#define MAX_GCIN_STR (256)

#define PHO_KBM "phokbm"

extern int win_xl, win_yl;
extern int win_x, win_y;
extern int dpy_xl, dpy_yl;

extern int gcin_font_size;

#define CHANGE_FONT_SIZE "change font size"
#define GCIN_FONT_SIZE "gcin-font-size"
