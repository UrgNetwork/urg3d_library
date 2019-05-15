@echo off

if not exist vsvars32.bat goto end

CALL vsvars32.bat

REM Compile URG library

cl.exe -c -MD -I../include ../src/urg3d_sensor.c
cl.exe -c -MD -I../include ../src/urg3d_connection.c
cl.exe -c -MD -I../include ../src/urg3d_tcpclient.c
cl.exe -c -MD -I../include ../src/urg3d_ring_buffer.c
cl.exe -c -MD -I../include ../src/urg3d_ticks.c

lib.exe /OUT:urg3d.lib urg3d_sensor.obj urg3d_connection.obj urg3d_tcpclient.obj urg3d_ring_buffer.obj urg3d_ticks.obj

REM Compile samples linking with ws2_32.lib setupapi.lib with /MD option.

cl.exe /MD -I../include ../samples/high_1_field_data_file.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/high_1_frame_4_field_data_file.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/high_1_frame_4_vfield_2_hfield_data_file.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/high_1_line_data_file.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/high_get_version.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/high_get_auxiliary.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/low_get_ax.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/low_get_multi_data.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/low_get_ri.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/low_get_ro.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/low_get_ro_with_error_reboot.c ws2_32.lib setupapi.lib urg3d.lib
cl.exe /MD -I../include ../samples/low_user_request.c ws2_32.lib setupapi.lib urg3d.lib

echo ビルドが完了しました。終了します。
set /p TMP=""
exit /b

:end
echo vsvars32.bat が見つかりません。終了します。
set /p TMP=""
exit /b
