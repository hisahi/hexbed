
FILES := bitedit.o inspector.o textconv.o

OBJS := $(OBJS) $(addprefix ui/tools/,$(FILES))
