#include "urg3d_sensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

urg3d_vssp_header_t header;
urg3d_range_header_t range_header;
urg3d_range_index_t range_index;
urg3d_data_range_t data_range;
char data[URG3D_MAX_RX_LENGTH];
int length_data;

void print_debug_info(const urg3d_t* urg, const urg3d_vssp_header_t* header, const urg3d_range_header_t* range_header, const urg3d_range_index_t* range_index,const urg3d_data_range_t* data_range)
{
    long i;
    printf("sizeof(*header) = %ld\n", (unsigned long)sizeof(*header));
    printf("header->mark = %.4s\n", header->mark);
    printf("header->type = %.3s\n", header->type);
    printf("header->status = %.3s\n", header->status);
    printf("header->header_length = %u\n", header->header_length);
    printf("header->length = %u\n", header->length);
    printf("header->received_time_ms = %lu\n", (long unsigned int)header->received_time_ms);
    printf("header->send_time_ms = %lu\n", (long unsigned int)header->send_time_ms);
    printf("sizeof(*range_header) = %ld\n", (unsigned long)sizeof(*range_header));
    printf("range_header->header_length = %u\n", range_header->header_length);
    printf("range_header->line_head_timestamp_ms = %lu\n", (long unsigned int)range_header->line_head_timestamp_ms);
    printf("range_header->line_tail_timestamp_ms = %lu\n", (long unsigned int)range_header->line_tail_timestamp_ms);
    printf("range_header->line_head_h_angle_ratio = %d\n", range_header->line_head_h_angle_ratio);
    printf("range_header->line_tail_h_angle_ratio = %d\n",range_header->line_tail_h_angle_ratio);
    printf("range_header->frame = %u\n", range_header->frame);
    printf("range_header->field = %u\n", range_header->h_field);
    printf("range_header->line = %u\n", range_header->line);
    printf("range_header->spot = %u\n", range_header->spot);
    printf("sizeof(*range_index) = %ld\n", (unsigned long)sizeof(*range_index));
    printf("range_index->index_length = %u\n", range_index->index_length);
    printf("range_index->nspots = %u\n", range_index->nspots);
    for(i = 0; i < range_index->nspots + 1; i++) {
        printf("range_index->index[%ld] = %u\n", i, range_index->index[i]);
    }
    printf("sizeof(*data_range) = %ld\n", (unsigned long)sizeof(*data_range));
    for(i = 0; i < range_index->index[range_index->nspots]; i++) {
        printf("data_range->raw[%ld].range_mm = %u\n",  i, data_range->raw[i].range_mm);
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
     * request to start the range only data
     */
    if((ret = urg3d_low_request_command(&urg, "DAT:ro=1\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send ok -> DAT:ro=1\n");
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
            if(urg3d_low_get_ro(&urg,  &header, &range_header, &range_index, &data_range) > 0) {
                print_debug_info(&urg,  &header, &range_header, &range_index, &data_range);
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
     * request to stop the range only data
     */
    if((ret = urg3d_low_request_command(&urg, "DAT:ro=0\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send ok -> DAT:ro=0\n");
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
