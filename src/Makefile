URG_CPP_LIB_STATIC = liburg3d.a
URG_CPP_LIB_SHARED = $(shell if test `echo $(OS) | grep Windows`; then echo "urg3d.dll"; else echo "liburg3d.so"; fi)

TARGET = $(URG_C_LIB_STATIC) $(URG_CPP_LIB_STATIC) \
		 $(URG_C_LIB_SHARED) $(URG_CPP_LIB_SHARED)

OBJ_CPP = Urg3dSensor.o Urg3dConnection.o \
		Urg3dRingBuffer.o Urg3dTcpclient.o

include ../build_rule.mk

CXXFLAGS = -g -O2 -I../include -fPIC -std=c++11
LDLIBS = -lm $(shell if test `echo $(OS) | grep Windows`; then echo "-lws2_32 -lsetupapi"; fi)

all : $(TARGET)

clean :
	$(RM) *.o $(TARGET) *.c~

$(URG_CPP_LIB_STATIC) : $(OBJ_CPP)
	$(AR) rv $@ $(OBJ_CPP)

$(URG_CPP_LIB_SHARED) : $(OBJ_CPP)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(OBJ_CPP) $(LDLIBS)

