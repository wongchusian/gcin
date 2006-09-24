#define MAX_CIN_PHR (32*CH_SZ + 1)

typedef struct {
  u_char key[4];   /* If I use u_long key, the struc size will be 8 */
  u_char ch[CH_SZ];
} ITEM;

typedef struct {
  u_char quick1[46][10][CH_SZ];
} QUICK_KEYS;

struct TableHead {
  char ename[16];         /* ascii name */
  char cname[16];         /* prompt */
  char selkey[12];        /* select keys */
  char endian;
  int KeyS;               /* number of keys needed */
  int MaxPress;           /* Max len of keystroke  ar30:4  changjei:5 */
  int M_DUP_SEL;          /* how many keys used to select */
  int DefC;               /* Defined characters */
  QUICK_KEYS qkeys;
};

u_long CONVT(char *s);
#define KeyBits (6)
#define MAX_GTAB_KEYS (1<<KeyBits)

#define MAX_GTAB_NUM_KEY 10
#define MAX_GTAB_ITEM_KEY_LEN (sizeof(((ITEM*)0)->key) * 8 / KeyBits)
#define MAX_SELKEY 16

typedef u_short gtab_idx1_t;

typedef struct {
  ITEM *tbl;
  QUICK_KEYS qkeys;
  int use_quick;
#define MAX_CNAME (4*CH_SZ+1)
  char cname[MAX_CNAME];
  u_char keycol[50];
  int KeyS;               /* number of keys needed */
  int MaxPress;           /* Max len of keystrike  ar30:4  changjei:5 */
  int DefChars;           /* defined chars */
  u_char keyname[MAX_GTAB_KEYS * CH_SZ];
  u_char *keyname_lookup; // used by boshiamy only
  gtab_idx1_t idx1[51];
  u_char keymap[MAX_GTAB_KEYS * CH_SZ];
  char selkey[MAX_SELKEY];
  u_char *sel1st;
  int M_DUP_SEL;
  int phrnum;
  int *phridx;
  char *phrbuf;
  char filename[16];
  time_t file_modify_time;
} INMD;

extern INMD inmd[MAX_GTAB_NUM_KEY+1];

typedef enum {
  GTAB_space_auto_first_any=1,    // boshiamy
  GTAB_space_auto_first_full=2,   // default behavior (cj, gcin's ar30)
  GTAB_space_auto_first_nofull=4  // windows ar30
} GTAB_space_pressed_E;

