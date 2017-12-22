#include "urg3d_sensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

urg3d_data_range_intensity_t data_range_intensity;
urg3d_data_ax_t ax_data;
urg3d_vssp_header_t header;
urg3d_range_header_t range_header;
urg3d_range_index_t range_index;
urg3d_ax_header_t ax_header;
char data[URG3D_MAX_RX_LENGTH];
int length_data;

int main(int argc, char *argv[])
{
    enum {
        CAPTURE_TIMES = 100 /*!< number of record to collect */
    };

    int n = 0; /* records count */
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
     * request to start the range and intensity data
     */
    if((ret = urg3d_low_request_command(&urg, "DAT:ri=1\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send ok -> DAT:ri=1\n");
    }

    /*
     * request to start the auxiliary data
     */
    if((ret = urg3d_low_request_command(&urg, "DAT:ax=1\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send ok -> DAT:ax=1\n");
    }

    /*
     * start main loop
     */
    printf("start loop\n");
    while(1) {
        /*
         * check received data
         */
        if(urg3d_next_receive_ready(&urg)) {
            /*
             * pick up the data (non-blocking)
             */
            if(urg3d_low_get_ri(&urg,  &header, &range_header, &range_index, &data_range_intensity) > 0) {
                printf("ri %d\n",n);
                n++;
            } else if(urg3d_low_get_ax(&urg, &header, &ax_header, &ax_data) > 0) {
                printf("ax %d\n",n);
                n++;
            } else if(urg3d_low_get_binary(&urg, &header, data, &length_data) > 0) {
                /*
                 * check error data
                 */
                if(strncmp(header.type, "ERR", 3) == 0 || strncmp(header.type, "_er", 3) == 0) {
                    printf("error %c%c%c %s", header.status[0], header.status[1], header.status[2], data);
                    if(header.status[0] != '0'){
                        break;
                    }
                }
            }
        } else {
            #ifdef URG3D_WINDOWS_OS
                Sleep(10);
            #else
                usleep(10000);
            #endif
        }
        /*
         * check capture times
         */
        if(n > CAPTURE_TIMES) {
            break;
        }
    }

    /*
     * request to stop the range and intensity data
     */
    if((ret = urg3d_low_request_command(&urg, "DAT:ri=0\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send ok -> DAT:ri=0\n");
    }

    /*
     * request to stop the auxiliary data
     */
    if((ret = urg3d_low_request_command(&urg, "DAT:ax=0\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send ok -> DAT:ax=0\n");
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
