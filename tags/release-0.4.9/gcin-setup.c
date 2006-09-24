#include "gcin.h"


static struct {
  char *name;
  char *kbm;
}  kbm_sel[]= {
 {"標準 standard", "zo"},
 {"標準 standard 使用 asdf 選擇", "zo-asdf"},
 {"倚天 Eten", "et"},
 {"倚天 Eten 使用 asdf 選擇", "et-asdf"},
 {"倚天 26 鍵", "et26"},
 {"倚天 26 鍵,使用 asdf 選擇", "et26-asdf"},
 {"許氏(國音,自然)", "hsu"},
 {"IBM", "ibm"},
};

static GtkWidget *check_button_phrase_pre_select, *check_button_gtab_dup_select_bell,
                 *check_button_gtab_full_space_auto_first,
                 *check_button_gtab_auto_select_by_phrase,
                 *check_button_gtab_pre_select;

static char *tsin_eng_ch_sw[]={
  "CapsLock",
  "Tab"
};
int tsin_eng_ch_swN = sizeof(tsin_eng_ch_sw) / sizeof(tsin_eng_ch_sw[0]);


static int kbm_selN = sizeof(kbm_sel) / sizeof(kbm_sel[0]);

static GtkWidget *gcin_kbm_window = NULL, *gcin_win0_conf_window;

static int new_select_idx, new_select_idx_tsin_sw;


static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
}

void save_gcin_conf_str(char *name, char *str);

static gboolean cb_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  FILE *fp;
  char fname[256];

  save_gcin_conf_str("phonetic-keyboard", kbm_sel[new_select_idx].kbm);
  save_gcin_conf_str("tsin-chinese-english-switch", tsin_eng_ch_sw[new_select_idx_tsin_sw]);
  save_gcin_conf_int("tsin-phrase-pre-select",
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phrase_pre_select)));

  send_gcin_message(GDK_DISPLAY(), "reload kbm");

  gtk_widget_destroy(gcin_kbm_window); gcin_kbm_window = NULL;
}


static void callback_button_clicked( GtkWidget *widget, gpointer data)
{
  new_select_idx = (int) data;
}


static void callback_button_clicked_tsin_sw( GtkWidget *widget, gpointer data)
{
  new_select_idx_tsin_sw = (int) data;
}


static int get_current_kbm_idx()
{
  char kbm_str[32];
  get_gcin_conf_str("phonetic-keyboard", kbm_str, "zo");

  int i;
  for(i=0; i < kbm_selN; i++)
    if (!strcmp(kbm_sel[i].kbm, kbm_str))
      return i;

  p_err("phonetic-keyboard->%s is not valid", kbm_str);

}

static int get_currnet_eng_ch_sw_idx()
{
  char swkey[32];
  get_gcin_conf_str("tsin-chinese-english-switch", swkey, "CapsLock");

  int i;
  for(i=0; i < kbm_selN; i++)
    if (!strcmp(tsin_eng_ch_sw[i], swkey))
      return i;

  p_err("tsin-chinese-english-switch->%s is not valid", swkey);
}

static gboolean get_current_tsin_phrase_pre_select()
{
  char phr_pre[16];

  get_gcin_conf_str("tsin-phrase-pre-select", phr_pre, "1");
  return atoi(phr_pre);
}

static gboolean close_kbm_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_kbm_window); gcin_kbm_window = NULL;
}


void create_kbm_window()
{
  load_setttings();

    gcin_kbm_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (G_OBJECT (gcin_kbm_window), "delete_event",
                      G_CALLBACK (close_kbm_window),
                      NULL);

    gtk_window_set_title (GTK_WINDOW (gcin_kbm_window), "gcin 注音鍵盤設定");
    gtk_container_set_border_width (GTK_CONTAINER (gcin_kbm_window), 3);

    GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER (gcin_kbm_window), vbox_top);

    GtkWidget *frame_kbm = gtk_frame_new("鍵盤排列方式");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_kbm, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_kbm), 3);

    GtkWidget *box_kbm = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame_kbm), box_kbm);
    gtk_container_set_border_width (GTK_CONTAINER (box_kbm), 3);

    int i;
    GSList *group_kbm = NULL;
    int current_idx = get_current_kbm_idx();
    new_select_idx = current_idx;

    for(i=0; i<kbm_selN; i++) {
      GtkWidget *button = gtk_radio_button_new_with_label (group_kbm, kbm_sel[i].name);
      gtk_box_pack_start (GTK_BOX (box_kbm), button, TRUE, TRUE, 0);

      group_kbm = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

      g_signal_connect (G_OBJECT (button), "clicked",
         G_CALLBACK (callback_button_clicked), (gpointer) i);

      if (i==current_idx)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    }

    GtkWidget *frame_tsin_sw = gtk_frame_new("詞音輸入[中/英]切換");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_sw, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_sw), 3);

    GtkWidget *box_tsin_sw = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame_tsin_sw), box_tsin_sw);
    gtk_container_set_border_width (GTK_CONTAINER (box_tsin_sw), 3);

    GSList *group_tsin_sw = NULL;
    current_idx = get_currnet_eng_ch_sw_idx();
    new_select_idx_tsin_sw = current_idx;

    for(i=0; i< tsin_eng_ch_swN; i++) {
      GtkWidget *button = gtk_radio_button_new_with_label (group_tsin_sw, tsin_eng_ch_sw[i]);
      gtk_box_pack_start (GTK_BOX (box_tsin_sw), button, TRUE, TRUE, 0);

      group_tsin_sw = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

      g_signal_connect (G_OBJECT (button), "clicked",
         G_CALLBACK (callback_button_clicked_tsin_sw), (gpointer) i);

      if (i==current_idx)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    }


    GtkWidget *frame_tsin_phrase_pre_select = gtk_frame_new("詞音輸入預選詞視窗");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_phrase_pre_select , TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_sw), 3);
    check_button_phrase_pre_select = gtk_check_button_new ();
    gtk_container_add (GTK_CONTAINER (frame_tsin_phrase_pre_select),
        check_button_phrase_pre_select);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_phrase_pre_select),
       get_current_tsin_phrase_pre_select());


    GtkWidget *button_close = gtk_button_new_with_label ("OK");
    gtk_box_pack_start (GTK_BOX (vbox_top), button_close, TRUE, TRUE, 0);

    g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                              G_CALLBACK (cb_ok),
                              G_OBJECT (gcin_kbm_window));
    GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button_close);

    gtk_widget_show_all (gcin_kbm_window);

    return;
}

static void cb_kbm()
{
  create_kbm_window();
}


static void cb_tslearn()
{
  system("tslearn &");
  exit(0);
}


static void cb_ret(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_destroy(user_data);
}


static void create_result_win(int res)
{
  char *restr = res ? "結果失敗":"結果成功";
  GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *button = gtk_button_new_with_label(restr);
  gtk_container_add (GTK_CONTAINER (main_window), button);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_ret), main_window);

  gtk_widget_show_all(main_window);
}



static cb_file_ts_export(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   char gcin_dir[512];

   get_gcin_dir(gcin_dir);

   char cmd[256];
   snprintf(cmd, sizeof(cmd), "tsd2a %s/tsin > %s", gcin_dir, selected_filename);
   int res = system(cmd);
   create_result_win(res);
}

static void cb_ts_export()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new ("請輸入要匯出的檔案名稱");

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_export),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
}

static cb_file_ts_import(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   char cmd[256];
   snprintf(cmd, sizeof(cmd),
      "cd %s/.gcin && tsd2a tsin > tmpfile && cat %s >> tmpfile && tsa2d tmpfile",
      getenv("HOME"), selected_filename);
   int res = system(cmd);
   create_result_win(res);
}

static void cb_ts_import()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new ("請輸入要匯入的檔案名稱");

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_import),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
}


static void cb_ts_edit()
{
  system("( cd ~/.gcin && tsd2a tsin > tmpfile && gedit tmpfile && tsa2d tmpfile ) &");
}


static void cb_help()
{
  char cmd[512];

  sprintf(cmd, "gedit %s/README &", DOC_DIR);
  system(cmd);
}


static GtkWidget *spinner;

static gboolean cb_win0_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{

  int font_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner));

  save_gcin_conf_int(GCIN_FONT_SIZE, font_size);

  save_gcin_conf_int(GTAB_DUP_SELECT_BELL,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell)));

  save_gcin_conf_int(GTAB_FULL_SPACE_AUTO_FIRST,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_full_space_auto_first)));

  save_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_auto_select_by_phrase)));

  save_gcin_conf_int(GTAB_PRE_SELECT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select)));

  send_gcin_message(GDK_DISPLAY(), CHANGE_FONT_SIZE);
  gtk_widget_destroy(gcin_win0_conf_window); gcin_win0_conf_window = NULL;
}

static gboolean close_win0_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_win0_conf_window); gcin_win0_conf_window = NULL;
}


void create_win0_conf_window()
{
  load_setttings();

    gcin_win0_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (G_OBJECT (gcin_win0_conf_window), "delete_event",
                      G_CALLBACK (close_win0_conf_window),
                      NULL);

    gtk_window_set_title (GTK_WINDOW (gcin_win0_conf_window), "輸入視窗外觀設定");
    gtk_container_set_border_width (GTK_CONTAINER (gcin_win0_conf_window), 3);

    GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER (gcin_win0_conf_window), vbox_top);

    GtkWidget *frame_font_size = gtk_frame_new("字型大小");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_font_size, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_font_size), 3);


    GtkAdjustment *adj =
     (GtkAdjustment *) gtk_adjustment_new (gcin_font_size, 8.0, 24.0, 1.0, 1.0, 0.0);

    spinner = gtk_spin_button_new (adj, 0, 0);
    gtk_container_add (GTK_CONTAINER (frame_font_size), spinner);


    GtkWidget *frame_gtab = gtk_frame_new("倉頡/行列/嘸蝦米/大易");
    gtk_container_set_border_width (GTK_CONTAINER (frame_gtab), 5);
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_gtab, FALSE, FALSE, 0);
    GtkWidget *vbox_gtab = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER (frame_gtab), vbox_gtab);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_gtab), 10);


    GtkWidget *hbox_gtab_dup_select_bell = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_dup_select_bell, FALSE, FALSE, 0);
    GtkWidget *label_gtab_sele = gtk_label_new("重複字選擇鈴聲");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell), label_gtab_sele,  FALSE, FALSE, 0);
    check_button_gtab_dup_select_bell = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell),check_button_gtab_dup_select_bell,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell),
       gtab_dup_select_bell);


    GtkWidget *hbox_gtab_full_space_auto_first = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_full_space_auto_first, FALSE, FALSE, 0);
    GtkWidget *label_gtab_full_space = gtk_label_new("按滿空白自動選擇第一個");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_full_space_auto_first), label_gtab_full_space,  FALSE, FALSE, 0);
    check_button_gtab_full_space_auto_first = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_full_space_auto_first),check_button_gtab_full_space_auto_first,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_full_space_auto_first),
       gtab_full_space_auto_first);


    GtkWidget *hbox_gtab_auto_select_by_phrase = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_auto_select_by_phrase, FALSE, FALSE, 0);
    GtkWidget *label_gtab_auto_select = gtk_label_new("自動由詞庫自動選擇字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), label_gtab_auto_select,  FALSE, FALSE, 0);
    check_button_gtab_auto_select_by_phrase = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase),check_button_gtab_auto_select_by_phrase,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_auto_select_by_phrase),
       gtab_auto_select_by_phrase);


    GtkWidget *hbox_gtab_pre_select = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_pre_select, FALSE, FALSE, 0);
    GtkWidget *label_gtab_pre_select = gtk_label_new("預覽/預選 字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select), label_gtab_pre_select,  FALSE, FALSE, 0);
    check_button_gtab_pre_select = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select),check_button_gtab_pre_select,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select),
       gtab_pre_select);


    GtkWidget *button_close = gtk_button_new_with_label ("OK");
    gtk_box_pack_start (GTK_BOX (vbox_top), button_close, TRUE, TRUE, 0);

    g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                              G_CALLBACK (cb_win0_conf_ok),
                              G_OBJECT (gcin_kbm_window));

    GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button_close);

    gtk_widget_show_all (gcin_win0_conf_window);

    return;
}

static void cb_win0_conf()
{
  create_win0_conf_window();
}


static void cb_default_input_method()
{
  create_gtablist_window();
}


static void create_main_win()
{
  GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (close_application),
                     NULL);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  GtkWidget *button_kbm = gtk_button_new_with_label("gcin 注音輸入設定");
  gtk_box_pack_start (GTK_BOX (vbox), button_kbm, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_kbm), "clicked",
                    G_CALLBACK (cb_kbm), NULL);

  GtkWidget *button_win0_conf = gtk_button_new_with_label("外觀鈴聲行為設定");
  gtk_box_pack_start (GTK_BOX (vbox), button_win0_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_win0_conf), "clicked",
                    G_CALLBACK (cb_win0_conf), NULL);
  GtkWidget *button_default_input_method = gtk_button_new_with_label("內定輸入法 & 開啟/關閉");
  gtk_box_pack_start (GTK_BOX (vbox), button_default_input_method, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_default_input_method), "clicked",
                    G_CALLBACK (cb_default_input_method), NULL);

  GtkWidget *button_tslearn = gtk_button_new_with_label("讓詞音從文章學習詞");
  gtk_box_pack_start (GTK_BOX (vbox), button_tslearn, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_tslearn), "clicked",
                    G_CALLBACK (cb_tslearn), NULL);

  GtkWidget *button_ts_export = gtk_button_new_with_label("詞庫匯出");
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_export, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label("詞庫匯入");
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_import, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label("詞庫編輯");
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_edit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);

  GtkWidget *button_help = gtk_button_new_with_label("使用說明");
  gtk_box_pack_start (GTK_BOX (vbox), button_help, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_help), "clicked",
                    G_CALLBACK (cb_help), NULL);

  gtk_widget_show_all(main_window);
}


int main(int argc, char **argv)
{
  init_TableDir();

  load_setttings();

  gtk_init(&argc, &argv);

  create_main_win();

  // once you invoke gcin-setup, the left-right buton tips is disabled
  save_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);

  gtk_main();
}