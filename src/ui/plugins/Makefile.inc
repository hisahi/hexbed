
FILES := export.o import.o inspector.o

OBJS := $(OBJS) $(addprefix ui/plugins/,$(FILES))
