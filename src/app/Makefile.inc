
FILES := config.o bitop.o encoding.o plugin.o

OBJS := $(OBJS) $(addprefix app/,$(FILES))
