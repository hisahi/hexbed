
FILES := uint8.o uint16.o uint32.o uint64.o \
         int8.o int16.o int32.o int64.o \
         float32.o float32x.o float64.o float64x.o

OBJS := $(OBJS) $(addprefix plugins/inspector/,$(FILES))
