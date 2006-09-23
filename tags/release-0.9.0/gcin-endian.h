#define swap_ch(a, b) do { char t; t = *(a); *(a) = *(b); *(b) = t; } while (0)

#if __BYTE_ORDER == __BIG_ENDIAN
#warning "big endian"
#define to_gcin_endian_2(pp) do { char *p=(char *)pp;  swap_ch(p, p+1); } while (0)
#define to_gcin_endian_4(pp) do { char *p=(char *)pp;  swap_ch(p, p+3); swap_ch(p+1, p+2); } while (0)
#else
#define to_gcin_endian_2(pp) do { } while (0)
#define to_gcin_endian_4(pp) do { } while (0)
#define to_gcin_endian_8(pp) do { } while (0)
#endif
