#include "gcin.h"

main()
{
  gdk_init(NULL, NULL);
  send_gcin_message(GDK_DISPLAY(), GB_OUTPUT_TOGGLE);

  return 0;
}
