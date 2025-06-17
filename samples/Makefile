TARGET = LowUserRequest HighGetInformation High1LineDataFile High1FieldDataFile High1Frame4FieldDataFile High1Frame4MotorField2RemFieldDataFile HighGetAuxiliary HighGetSpotData High1FieldAugmentedDataFile

URG3D_LIB = ../src/liburg3d.a

include ../build_rule.mk

CXXFLAGS = -O2 $(INCLUDES) -I../include -std=c++11
LDLIBS = -lm $(shell if test `uname | grep Linux`; then echo "-lrt"; fi) `/bin/sh ld_ws2_32.sh` `/bin/sh ld_setupapi.sh`

all : $(TARGET)

clean :
	$(RM) *.o $(TARGET) *.exe *.cpp~

$(TARGET) : $(URG3D_LIB)

$(URG3D_LIB) :
	cd $(@D)/ && $(MAKE) $(@F)
