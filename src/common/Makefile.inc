
FILES := charconv.o config.o floatconv.o hexconv.o logger.o memory.o values.o

OBJS := $(OBJS) $(addprefix common/,$(FILES))
