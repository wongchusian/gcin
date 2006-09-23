#include <pwd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gcin.h"

char *get_gcin_xim_name();

void get_gcin_im_srv_sock_path(char *outstr, int outstrN)
{
  char *disp = getenv("DISPLAY");
  int my_uid = getuid();

  if (!disp || !strcmp(disp, ":0"))
    disp = ":0.0";

  char tdisp[64];
  strcpy(tdisp, disp);

  if (!strchr(disp, ':'))
      strcat(tdisp, ":0");
  if (!strchr(disp, '.'))
      strcat(tdisp, ".0");

  char my_dir[128];

  struct passwd *pw = getpwuid(my_uid);
  snprintf(my_dir, sizeof(my_dir), "/tmp/gcin-%s", pw->pw_name);
  struct stat st;

  if (stat(my_dir, &st) < 0)
    mkdir(my_dir, 0700);
  else {
    if (st.st_uid != my_uid) {
      fprintf(stderr, "please check the permision of dir %s\n", my_dir);
      return;
    }
  }

  snprintf(outstr,outstrN, "%s/socket-%s-%s", my_dir, tdisp, get_gcin_xim_name());
}


Atom get_gcin_addr_atom(Display *dpy)
{
  if (!dpy) {
    dbg("dpy is null\n");
    return 0;
  }

  char *xim_name = get_gcin_xim_name();
  char tt[128];

  snprintf(tt, sizeof(tt), "GCIN_ADDR_ATOM_%s", xim_name);

  Atom atom = XInternAtom(dpy, tt, False);

  return atom;
}
