
FILES := export.o import.o inspector.o

OBJS := $(OBJS) $(addprefix plugins/,$(FILES))
