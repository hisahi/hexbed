
FILES := intelhex.o

OBJS := $(OBJS) $(addprefix ui/plugins/export/,$(FILES))
