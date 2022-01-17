
FILES := intelhex.o srec.o

OBJS := $(OBJS) $(addprefix plugins/import/,$(FILES))
