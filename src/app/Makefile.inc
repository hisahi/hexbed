
FILES := config.o bitop.o

OBJS := $(OBJS) $(addprefix app/,$(FILES))
