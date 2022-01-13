
FILES := intelhex.o

OBJS := $(OBJS) $(addprefix ui/plugins/import/,$(FILES))
