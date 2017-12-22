#include "urg3d_sensor.h"
#include <stdio.h>
#include <stdlib.h>

urg3d_vssp_header_t header;
urg3d_sensor_version_t version;

int main(int argc, char *argv[])
{
    int ret = 0; /* operation return */
    const char* const device = "192.168.0.10"; /* device ip address */
    const long port = 10940; /* device port number. It is a fixed value */
    urg3d_t urg;

    /*
     * open the connection to the sensor
     */
    if((ret = urg3d_open(&urg,  device, port)) < 0) {
        printf("error urg3d_open %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("open ok\n");
    }

    /*
     * set blocking function timeout = 2000[ms]
     */
    urg3d_high_set_blocking_timeout_ms(&urg, 2000);

    /*
     * receive version information from device
     */
    if((ret = urg3d_high_blocking_get_sensor_version(&urg, &version)) < 0) {
        printf("error urg3d_high_blocking_get_sensor_version %d\n", ret);
    }else{
        printf("version.vendor = %s\n", version.vendor);
        printf("version.product = %s\n", version.product);
        printf("version.serial = %s\n", version.serial);
        printf("version.firmware = %s\n", version.firmware);
        printf("version.protocol = %s\n", version.protocol);
    }

    /*
     * close the connection to the sensor
     */
    if((ret = urg3d_close(&urg)) < 0) {
        printf("error urg3d_close %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("close ok\n");
    }

#if defined(URG3D_MSC)
    getchar();
#endif
    return 0;
}
