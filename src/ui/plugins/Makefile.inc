
FILES := inspector.o plugin.o

OBJS := $(OBJS) $(addprefix ui/plugins/,$(FILES))
