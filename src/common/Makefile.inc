
FILES := charconv.o ctype.o config.o floatconv.o hexconv.o logger.o memory.o \
         random.o values.o

OBJS := $(OBJS) $(addprefix common/,$(FILES))
