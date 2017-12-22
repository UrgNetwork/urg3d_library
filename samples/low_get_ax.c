#include "urg3d_sensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

urg3d_vssp_header_t header;
urg3d_ax_header_t ax_header;
urg3d_data_ax_t ax_data;
char data[URG3D_MAX_RX_LENGTH];
int length_data;

void print_debug_info( const urg3d_t* urg, const urg3d_vssp_header_t* header, const urg3d_ax_header_t* ax_header, const urg3d_data_ax_t* ax_data)
{
    int i, count = 0;
    printf("sizeof(*header) = %ld\n", (unsigned long)sizeof(*header));
    printf("header->mark = %.4s\n", header->mark);
    printf("header->type = %.3s\n", header->type);
    printf("header->status = %.3s\n", header->status);
    printf("header->header_length = %u\n", header->header_length);
    printf("header->length = %u\n", header->length);
    printf("header->received_time_ms = %u\n", header->received_time_ms);
    printf("header->send_time_ms = %u\n", header->send_time_ms);
    printf("sizeof(*ax_header) = %ld\n", (unsigned long)sizeof(*ax_header));
    printf("ax_header->header_length = %u\n", ax_header->header_length);
    printf("ax_header->timestamp_ms = %u\n", ax_header->timestamp_ms);
    printf("ax_header->data_bitfield = 0x%x\n", ax_header->data_bitfield);
    printf("ax_header->data_count = %d\n",ax_header->data_count);
    printf("ax_header->data_ms = %d\n", ax_header->data_ms);
    printf("sizeof(*ax_data) = %ld\n", (unsigned long)sizeof(*ax_data));
    count = (header->length - header->header_length - ax_header->header_length) / sizeof(signed long);
    for(i = 0; i < count; i++) {
        printf("ax_data->value[%d] = %d\n", i, ax_data->value[i]);
    }
    printf("\n");
}

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
            if(urg3d_low_get_ax(&urg, &header, &ax_header, &ax_data) > 0) {
                print_debug_info(&urg, &header, &ax_header, &ax_data);
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
