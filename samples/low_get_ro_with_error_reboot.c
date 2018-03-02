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

static void print_captured_line_index(const urg3d_t *urg,
                                      int captured_count,
                                      const urg3d_range_header_t *range_header, const urg3d_range_index_t *range_index,
                                      const urg3d_data_range_t *data_range)
{

    printf("%d th line Frame %03d Field %01d Line %02d Spots %02d ",
           captured_count, range_header->frame, range_header->h_field, range_header->line,
           range_index->nspots);

    if (range_index->nspots != 0) {
        printf("first spot range %d", data_range->raw[0].range_mm);
    }
    printf("\n");

    return;
}

static int reboot_yvt_x002(urg3d_t *urg, const char *device, long port,
                           unsigned int reconnection_wait_time_msec,
                           unsigned int restart_wait_time_msec)
{
    const char YVT_X002_REBOOT_COMMAND[] = "#reboot\n";
    int ret = 0;
    if((ret = urg3d_low_request_command(urg, YVT_X002_REBOOT_COMMAND)) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    }

    // disconnect sensor
    if((ret = urg3d_close(urg)) < 0) {
        printf("error urg3d_close %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    }

#ifdef URG3D_WINDOWS_OS
    Sleep(reconnection_wait_time_msec);
#else
    usleep(reconnection_wait_time_msec * 1000);
#endif

    // connect to sensor
    if((ret = urg3d_open(urg,  device, port)) < 0) {
        printf("error urg3d_open %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    }

#ifdef URG3D_WINDOWS_OS
    Sleep(restart_wait_time_msec);
#else
    usleep(restart_wait_time_msec * 1000);
#endif

    return 0;
}

int main(int argc, char *argv[])
{
    enum {
        // number of lines to capture
        NUMBER_OF_LINES_TO_CAPTURE = 50000,

        // time to wait reconnection after reboot of sensor
        YVT_X002_RECONNECT_WAIT_TIME_MSEC = 5000,

        // time to wait restart sensor after rebooting
        YVT_X002_RESTART_WAIT_TIME_MSEC = 35000,

    };

    // number of captured lines
    int captured_count = 0;

    // return value
    int ret = 0;

    // sensor ip address
    const char* const device = "192.168.0.10";
    // sensor port
    const long port = 10940;

    urg3d_t urg;

    // connect to sensor
    if((ret = urg3d_open(&urg,  device, port)) < 0) {
        printf("error urg3d_open %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("open success\n");
    }


    // start data streaming
    if((ret = urg3d_low_request_command(&urg, "DAT:ro=1\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send success -> DAT:ro=1\n");
    }

    printf("wait for %d lines capture\n", NUMBER_OF_LINES_TO_CAPTURE);
    while(1) {

        // wait for next receive event
        if(urg3d_next_receive_ready(&urg)) {

            // decode line data
            if(urg3d_low_get_ro(&urg,  &header, &range_header, &range_index, &data_range) > 0) {

                print_captured_line_index(&urg, captured_count,
                                          &range_header, &range_index, &data_range);
                ++captured_count;

            } else if(urg3d_low_get_binary(&urg, &header, data, &length_data) > 0) {

                // checking error
                if(strncmp(header.type, "_er", 3) == 0) {

                    fprintf(stderr, "error %c%c%c %s", header.status[0], header.status[1], header.status[2], data);

                    // check error type and if the error is system error, reboot sensor
                    if (header.status[0] == '2') {
                        fprintf(stderr, "Reboot YVT-X002 ");

                        ret = reboot_yvt_x002(&urg, device, port,
                                              YVT_X002_RECONNECT_WAIT_TIME_MSEC,
                                              YVT_X002_RESTART_WAIT_TIME_MSEC);

                        if (ret < 0) {
                            fprintf(stderr, "failed\n");
                            break;
                        }

                        fprintf(stderr, "success\n");

                        // restart data streaming
                        if((ret = urg3d_low_request_command(&urg, "DAT:ro=1\n")) < 0) {
                            printf("error urg3d_low_request_command %d\n", ret);
                            ret = urg3d_close(&urg);
#if defined(URG3D_MSC)
                            getchar();
#endif
                            return -1;
                        } else {
                            fprintf(stderr, "resend success -> DAT:ro=1\n");
                        }

                    } else if (header.status[0] == '1') {
                        printf("Request error\n");
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

        // if end capture sequence
        if(captured_count > NUMBER_OF_LINES_TO_CAPTURE) {
            break;
        }
    }


    // stop data streaming
    if((ret = urg3d_low_request_command(&urg, "DAT:ro=0\n")) < 0) {
        printf("error urg3d_low_request_command %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("send success -> DAT:ro=0\n");
    }

    // disconnect sensor
    if((ret = urg3d_close(&urg)) < 0) {
        printf("error urg3d_close %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("close success\n");
    }

#if defined(URG3D_MSC)
    getchar();
#endif
    return 0;
}
