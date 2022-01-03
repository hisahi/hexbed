
FILES := validate.o general.o appearance.o layout.o

OBJS := $(OBJS) $(addprefix ui/settings/,$(FILES))
