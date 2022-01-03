
FILES := radixpicker.o find.o goto.o replace.o selectblock.o

OBJS := $(OBJS) $(addprefix ui/dialogs/,$(FILES))
