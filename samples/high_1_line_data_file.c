#include "urg3d_sensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

urg3d_vssp_header_t header;
urg3d_measurement_data_t measurement_data;
char data[URG3D_MAX_RX_LENGTH];
int length_data;
int buffer_length = 0;
char buffer[1024*1024]; //1MB

void to_buffer(const urg3d_t* urg, const urg3d_measurement_data_t* measurement_data)
{
    int spot, echo;
    buffer_length += sprintf(buffer + buffer_length, "#x_m,#y_m,#z_m,#range_m,#vertical_rad,#horizontal_rad,#intensity,#spot(0-%d),#echo(0-max%d),#->frame:field:line->,%d,%d,%d\n"
      , measurement_data->spot_count-1, URG3D_MAX_ECHOS_COUNT-1
      , measurement_data->frame_number, measurement_data->h_field_number, measurement_data->line_number);
    for(spot=0; spot < measurement_data->spot_count; ++spot) {
        for(echo=0; echo < measurement_data->spots[spot].echo_count; ++echo) {
            buffer_length += sprintf(buffer + buffer_length, "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%d\n"
              , measurement_data->spots[spot].point[echo].x_m
              , measurement_data->spots[spot].point[echo].y_m
              , measurement_data->spots[spot].point[echo].z_m
              , measurement_data->spots[spot].polar[echo].range_m
              , measurement_data->spots[spot].polar[echo].vertical_rad
              , measurement_data->spots[spot].polar[echo].horizontal_rad
              , measurement_data->spots[spot].polar[echo].intensity
              , spot
              , echo);
        }
    }
}

int main(int argc, char *argv[])
{
    int ret = 0; /* operation return */
    const char* const device = "192.168.0.10"; /* device ip address */
    const long port = 10940; /* device port number. It is a fixed value */
    urg3d_t urg;
    FILE *fp;

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
     * initialize the urg3d session (get transform tables)
     */
    if((ret = urg3d_high_blocking_init(&urg)) < 0) {
        printf("error urg3d_high_blocking_init %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    }

    /*
     * start the acquisition mode. possible value are:
     *     URG3D_DISTANCE
     *     URG3D_DISTANCE_INTENSITY
     *     URG3D_AUXILIARY
     */
    if((ret = urg3d_high_start_data(&urg, URG3D_DISTANCE_INTENSITY)) < 0) {
        printf("error urg3d_high_start_data %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
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
            if(urg3d_high_get_measurement_data(&urg, &measurement_data) > 0) {
                /*
                 * add data for buffer, and break the loop
                 */
                to_buffer(&urg, &measurement_data);
                break;
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
    }

    /*
     * save csv format file
     */
    fp = fopen("output.csv", "w");
    fwrite(buffer, sizeof(char), buffer_length, fp);
    fclose(fp);
    printf("save output.csv\n");

    /*
     * stop the acquisition flow started earlier using the start acquisition mode.
     */
    if((ret = urg3d_high_stop_data(&urg, URG3D_DISTANCE_INTENSITY)) < 0) {
        printf("error urg3d_high_stop_data %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
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
