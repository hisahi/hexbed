
FILES := bitedit.o inspector.o

OBJS := $(OBJS) $(addprefix ui/tools/,$(FILES))
