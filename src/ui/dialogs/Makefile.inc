
FILES := radixpicker.o bitopbinary.o bitopshift.o bitopunary.o find.o \
         goto.o insert.o random.o replace.o selectblock.o
OBJS := $(OBJS) $(addprefix ui/dialogs/,$(FILES))
