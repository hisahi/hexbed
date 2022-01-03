
FILES := config.o

OBJS := $(OBJS) $(addprefix app/,$(FILES))
