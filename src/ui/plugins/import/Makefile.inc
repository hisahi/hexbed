
FILES := intelhex.o srec.o

OBJS := $(OBJS) $(addprefix ui/plugins/import/,$(FILES))
