
FILES := treble.o task.o document.o search.o cisearch.o \
         bnew.o bfile.o

OBJS := $(OBJS) $(addprefix file/,$(FILES))
