
FILES := menus.o menufile.o menuedit.o menuview.o menusearch.o menuhelp.o \
         clipboard.o encoding.o status.o logger.o applock.o toolbar.o \
         hexedit.o textinput.o context.o config.o editor.o saeditor.o \
         settings.o hexbed.o

OBJS := $(OBJS) $(addprefix ui/,$(FILES))
