#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#define GETTEXT_PACKAGE "gcin"
#include <glib/gi18n-lib.h>
#include "eggtrayicon.h"

static GtkWidget *image;

static void
cb_pref (GtkAction * action)
{
  system(GCIN_BIN_DIR"/gcin-setup &");
}

char *gcin_icons[]={SYS_ICON_DIR"/gcin-tray.png", SYS_ICON_DIR"/gcin-tray-sim.png"};


void toggle_gb_output();
extern gboolean gb_output;

static void cb_trad_sim_toggle(GtkAction * action)
{
  toggle_gb_output();
  gtk_image_set_from_file(GTK_IMAGE(image), gcin_icons[gb_output]);
}

static void cb_sim2trad(GtkAction * action)
{
  system(GCIN_BIN_DIR"/sim2trad &");
}


static void cb_trad2sim(GtkAction * action)
{
  system(GCIN_BIN_DIR"/trad2sim &");
}

static GtkActionEntry entries[] = {
  {"Preferences", GTK_STOCK_PREFERENCES,
   N_("_Preferences"), "<control>P",
   N_("Preferences"),
   G_CALLBACK (cb_pref)},

   {"trad sim toggle", NULL,
   N_("trad sim"),
   G_CALLBACK (cb_trad_sim_toggle)},

  {"trad2sim", NULL,
   N_("trad2sim"), "<control>S",
   N_("_trad2sim"),
   G_CALLBACK (cb_trad2sim)},


  {"sim2trad", NULL,
   N_("sim2trad"), "<control>T",
   N_("_sim2trad"),
   G_CALLBACK (cb_sim2trad)},
};

static const gchar *ui_info =
  "<ui>"
  "  <popup name='PopupMenu'>"
  "    <menuitem action='Preferences'/>"
  "    <menuitem action='trad2sim'/>"
  "    <menuitem action='sim2trad'/>"
  "  </popup>"
  "</ui>";


static GtkWidget *menu;

static void create_menu()
{
#if 0
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  GtkActionGroup *actions = gtk_action_group_new ("Actions");
  gtk_action_group_set_translation_domain (actions, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (actions, entries, G_N_ELEMENTS (entries), NULL);
  GtkUIManager *ui = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (ui, actions, 0);

  GError *error = NULL;
  if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error)) {
    g_message ("building menus failed: %s", error->message);
    g_error_free (error);
  }

  menu = gtk_ui_manager_get_widget (ui, "/PopupMenu");

  return;
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

gboolean
tray_button_press_event_cb (GtkWidget * button, GdkEventButton * event, gpointer userdata)
{
  switch (event->button) {
    case 1:
      cb_trad_sim_toggle(NULL);
      break;
    case 2:
      inmd_switch_popup_handler(NULL, event);
      break;
    case 3:
      if (!menu)
        create_menu();

      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
         event->button, event->time);
      break;
  }

  return TRUE;
}

void create_tray()
{
  EggTrayIcon *tray_icon = egg_tray_icon_new ("gcin");
  GtkWidget *event_box = gtk_event_box_new ();

  gtk_container_add (GTK_CONTAINER (tray_icon), event_box);

  g_signal_connect (G_OBJECT (event_box), "button-press-event",
                    G_CALLBACK (tray_button_press_event_cb), NULL);

  image = gtk_image_new_from_file(gcin_icons[gb_output]);


  gtk_container_add (GTK_CONTAINER (event_box), image);

  gtk_widget_show_all (GTK_WIDGET (tray_icon));
}
