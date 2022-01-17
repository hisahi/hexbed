
FILES := carray.o csharp.o html.o java.o intelhex.o srec.o text.o

OBJS := $(OBJS) $(addprefix plugins/export/,$(FILES))
