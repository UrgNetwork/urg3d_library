TARGET = low_get_ri low_get_ro low_get_ax low_get_multi_data low_user_request high_get_version high_1_line_data_file high_1_field_data_file high_1_frame_4_field_data_file high_1_frame_4_vfield_2_hfield_data_file high_get_auxiliary low_get_ro_with_error_reboot

URG3D_LIB = ../src/liburg3d.a

include ../build_rule.mk

CFLAGS = -O2 $(INCLUDES) -I../include
LDLIBS = -lm $(shell if test `uname | grep Linux`; then echo "-lrt"; fi) `/bin/sh ld_wsock.sh` `/bin/sh ld_setupapi.sh`

all : $(TARGET)

clean :
	$(RM) *.o $(TARGET) *.exe *.c~

$(TARGET) : $(URG3D_LIB)

$(URG3D_LIB) :
	cd $(@D)/ && $(MAKE) $(@F)
