
FILES := export.o import.o inspector.o plugin.o

OBJS := $(OBJS) $(addprefix ui/plugins/,$(FILES))
