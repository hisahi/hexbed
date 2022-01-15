
FILES := config.o bitop.o plugin.o sbcs.o

OBJS := $(OBJS) $(addprefix app/,$(FILES))
