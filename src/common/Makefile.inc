
FILES := charconv.o config.o hexconv.o logger.o memory.o values.o

OBJS := $(OBJS) $(addprefix common/,$(FILES))
