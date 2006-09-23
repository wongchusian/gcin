OPTFLAGS=-g

include config.mak

.SUFFIXES:	.c .o .E

OBJS=gcin.o IC.o eve.o win0.o pho.o tsin.o win1.o util.o pho-util.o gcin-conf.o tsin-util.o \
     win-sym.o intcode.o pho-sym.o win-int.o win-pho.o gcin-settings.o table-update.o
OBJS_TSLEARN=tslearn.o util.o gcin-conf.o pho-util.o tsin-util.o gcin-send.o pho-sym.o \
             table-update.o
OBJS_phod2a=phod2a.o pho-util.o gcin-conf.o pho-sym.o table-update.o
OBJS_tsa2d=tsa2d.o gcin-send.o util.o pho-sym.o gcin-conf.o
OBJS_phoa2d=phoa2d.o pho-sym.o gcin-send.o gcin-conf.o
OBJS_kbmcv=kbmcv.o pho-sym.o
OBJS_tsd2a=tsd2a.o pho-sym.o
OBJS_gcin_steup=gcin-setup.o gcin-conf.o util.o gcin-send.o gcin-settings.o
CFLAGS= $(OPTFLAGS) $(GTKINC) -I./IMdkit/include -DDEBUG="0$(GCIN_DEBUG)" \
        -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\"  -DDOC_DIR=\"$(DOC_DIR)\" \
        -DGCIN_ICON_DIR=\"$(GCIN_ICON_DIR)\"

IMdkitLIB = IMdkit/lib/libXimd.a

.c.E:
	$(CC) $(CFLAGS) -E -o $@ $<

PROGS=gcin tsd2a tsa2d phoa2d phod2a tslearn gcin-setup
PROGS_CV=kbmcv

all:	$(PROGS) $(DATA) $(PROGS_CV) gcin.spec
	make -C data

gcin:      $(OBJS) $(IMdkitLIB)
	$(CC) -o $@ $(OBJS) $(IMdkitLIB) $(LDFLAGS)
	rm -f core.*
	ln -sf $@ $@.test

tslearn:        $(OBJS_TSLEARN)
	cc -o $@ $(OBJS_TSLEARN) $(LDFLAGS)

gcin-setup:     $(OBJS_gcin_steup)
	cc -o $@ $(OBJS_gcin_steup) $(LDFLAGS)

phoa2d: $(OBJS_phoa2d)
	cc -o $@ $(OBJS_phoa2d) $(LDFLAGS)

phod2a: $(OBJS_phod2a)
	cc -o $@ $(OBJS_phod2a) $(LDFLAGS)

tsa2d:  $(OBJS_tsa2d)
	cc -o $@ $(OBJS_tsa2d) $(LDFLAGS)

tsd2a:  $(OBJS_tsd2a)
	cc -o $@ $(OBJS_tsd2a) $(LDFLAGS)

kbmcv:  $(OBJS_kbmcv)
	$(CC) -o $@ $(OBJS_kbmcv)

$(IMdkitLIB):
	make -C IMdkit/lib

install:
	install -d $(GCIN_TABLE_DIR)
	install -d $(datadir)/icons
	install gcin.png $(datadir)/icons
	install -d $(GCIN_ICON_DIR)
	install -m 644 icons/* $(GCIN_ICON_DIR)
	install -d $(bindir)
	install -d $(libdir)/menu
	install -m 644 menu/* $(libdir)/menu
	make -C data install
	if [ $(prefix) = /usr/local ]; then \
	   install -m 644 menu/* /usr/lib/menu; \
	   which update-menus >& /dev/null && update-menus; \
	   sh modify-XIM; \
	   install -d $(DOC_DIR); \
	   install -m 644 README $(DOC_DIR); \
	   install $(PROGS) $(bindir); \
	else \
	   install -m 644 menu/* $(libdir)/menu; \
	   install -s $(PROGS) $(bindir); \
	fi
clean:
	make -C IMdkit clean
	make -C data clean
	rm -f *.o *~ *.E config.mak tags core.* $(PROGS) $(PROGS_CV) $(DATA) .depend gcin.spec menu/*~
	cd ..; tar cvfj gcin.tbz gcin

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

config.mak: VERSION.gcin configure
	./configure

gcin.spec:	gcin.spec.in
	sed -e "s/__gcin_version__/$(GCIN_VERSION)/" < $< > $@

include .depend