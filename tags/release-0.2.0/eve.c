#include "gcin.h"

#define STRBUFLEN 64

extern Display *dpy;
extern XIMS current_ims;
static IMForwardEventStruct *current_forward_eve;

InputStyle_E current_input_style;
IC *current_IC;

void init_in_method(int in_no);

void set_current_input_style(InputStyle_E style)
{
  current_input_style = style;
}


void send_text(char *text)
{
  XTextProperty tp;
#if DEBUG
  dbg("sendkey_b5: %s\n", text);
#endif
  XmbTextListToTextProperty(dpy, &text, 1, XCompoundTextStyle, &tp);

  ((IMCommitStruct*)current_forward_eve)->flag |= XimLookupChars;
  ((IMCommitStruct*)current_forward_eve)->commit_string = tp.value;
  IMCommitString(current_ims, (XPointer)current_forward_eve);

  XFree(tp.value);
}


void sendkey_b5(char *bchar)
{
  char tt[3];

  tt[0]=bchar[0];
  tt[1]=bchar[1];
  tt[2]=0;

  send_text(tt);
}

static bounce_back_key()
{
    IMForwardEventStruct forward_ev = *(current_forward_eve);
    IMForwardEvent(current_ims, (XPointer)&forward_ev);
}

void hide_win0();

void hide_in_win(IC *ic)
{
  if (!ic) {
#if DEBUG
    dbg("hide_in_win: ic is null");
#endif
    return;
  }

  switch (ic->in_method) {
    case 3:
      hide_win_pho();
      break;
    case 6:
//      flush_tsin_buffer();
      hide_win0();
      break;
    case 10:
      hide_win_int();
      break;
  }
}


void show_in_win(IC *ic)
{
  if (!ic) {
#if DEBUG
    dbg("show_in_win: ic is null");
#endif
    return;
  }

  switch (ic->in_method) {
    case 3:
      show_win_pho();
      break;
    case 6:
      show_win0();
      break;
    case 10:
      show_win_int();
      break;
  }
}

void move_in_win(IC *ic, int x, int y)
{
  if (!ic) {
#if DEBUG
    dbg("move_in_win: ic is null");
#endif
    return;
  }

  switch (ic->in_method) {
    case 3:
      move_win_pho(x, y);
      break;
    case 6:
      move_win0(x, y);
      break;
    case 10:
      move_win_int(x, y);
      break;
  }
}

void move_IC_win0(IC *rec);

void update_in_win_pos()
{
  if (current_input_style == InputStyleRoot) {
    Window r_root, r_child;
    int winx, winy, rootx, rooty;
    u_int mask;

    XQueryPointer(dpy, root, &r_root, &r_child, &rootx, &rooty, &winx, &winy, &mask);

    move_in_win(current_IC, winx+1, winy+1);
  } else {
    if (current_IC)
      move_IC_in_win(current_IC);
  }
}

extern IMProtocol *current_call_data;
IC *findIC();

void toggle_im_enabled()
{
//    dbg("toggle_im_enabled\n");

    if (current_IC->b_im_enabled) {

      if (current_IC->in_method)
        flush_tsin_buffer();

      hide_in_win(current_IC);
      current_IC->b_im_enabled = FALSE;
    } else {
      current_IC->b_im_enabled = TRUE;

      if (!current_IC->in_method) {
        init_in_method(6);
      }

      update_in_win_pos();

      show_in_win(current_IC);
    }
}

void update_active_in_win_geom()
{
  switch (current_IC->in_method) {
    case 6:
      get_win0_geom();
      break;
  }
}

void toggle_half_full_char()
{
  current_IC->b_half_full_char ^= 1;
//  dbg("half full toggle\n");
}

void init_in_method(int in_no)
{

  if (current_IC->in_method != in_no)
    hide_in_win(current_IC);

//  dbg("switch init_in_method %d\n", in_no);
  switch (in_no) {
    case 3:
      current_IC->in_method = 3;
      init_tab_pho(True);
      break;
    case 6:
      current_IC->in_method = 6;
      init_tab_pp(True);
      break;
    case 0:
      current_IC->in_method = 10;
      init_inter_code(True);
      break;
  }

  update_in_win_pos();
}


gboolean feedkey_pho(KeySym xkey);
gboolean feedkey_pp(KeySym xkey, int state);

int ProcessKey()
{
  char strbuf[STRBUFLEN];
  KeySym keysym;

  memset(strbuf, 0, STRBUFLEN);
  XKeyEvent *kev = (XKeyEvent*)&current_forward_eve->event;
  int count = XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);
#if 0
  if (count > 0) {
      dbg("count:%d [%s] ", count, strbuf);
  }
#endif

  if (keysym == XK_space) {
    if (kev->state & ControlMask) {
      if (current_IC->in_method == 6)
        tsin_toggle_eng_ch(1);
      toggle_im_enabled();
      return;
    }
    else
    if (kev->state & ShiftMask) {
      if (current_IC->in_method == 6)
        tsin_toggle_half_full();
      else
        toggle_half_full_char();

      return;
    }
  }

  if (!current_IC->b_im_enabled) {
    bounce_back_key();
    return;
  }

  int status = 0;

  if ((kev->state & ControlMask) && (kev->state&(GDK_MOD1_MASK|GDK_MOD5_MASK))) {
    init_in_method(keysym- XK_0);
    return;
  }

  switch(current_IC->in_method) {
    case 3:
      status = feedkey_pho(keysym);
      break;
    case 6:
      status = feedkey_pp(keysym, kev->state);
      break;
    case 10:
      status = feedkey_intcode(keysym, kev->state);
      break;
  }

  if (!status)
    bounce_back_key();
}


int gcin_ForwardEventHandler(IMForwardEventStruct *call_data)
{
    current_forward_eve = call_data;

    if (call_data->event.type != KeyPress) {
#if DEBUG
        dbg("bogus event type, ignored\n");
#endif
    	return True;
    }

    ProcessKey(call_data);
}

IC *FindIC(CARD16 icid);

int gcin_FocusIn(IMChangeFocusStruct *call_data)
{
    IC *ic = FindIC(call_data->icid);

    if (ic)
      load_IC(ic);

#if DEBUG
    dbg("focus in %d\n", call_data->icid);
#endif
    return True;
}

int gcin_FocusOut(IMChangeFocusStruct *call_data)
{
    IC *ic = FindIC(call_data->icid);

    if (ic == current_IC) {
      hide_win0();
    }
#if DEBUG
    dbg("focus out %d\n", call_data->icid);
#endif
    return True;
}

