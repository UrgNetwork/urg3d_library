#include "urg3d_sensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

urg3d_vssp_header_t header;
char data[URG3D_MAX_RX_LENGTH];
int length_data;

enum YVT_X002_AUXILIARY_DATA_INDEX {
    ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS = 0,
    ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS,
    ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS,

    ACCELERATION_IN_X_DIRECTION,
    ACCELERATION_IN_Y_DIRECTION,
    ACCELERATION_IN_Z_DIRECTION,

    TEMPERATURE,

    NUMBER_OF_YVT_X002_AUXILIARY_DATA,
};

#define MAXIMUM_RECORD_TIMES 10
#define MAXIMUM_CAPTURING_COUNT 300

double yvt_sensor_ax_data_timestamps[MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES];
double yvt_sensor_scaled_ax_data[MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES][NUMBER_OF_YVT_X002_AUXILIARY_DATA];

//! following YVT-X002 scaling factors are defined in MPU-9250 product specification

//! scaling factor for angular velocity (500.0 * 0.001 / 32767) [deg/msec]
#define YVT_X002_ANGULAR_VELOCITY_SCALING_FACTOR 0.00001525925

//! scaling factor for acceleration (1 / 8192) [G]
#define YVT_X002_ACCELERATION_SCALING_FACTOR 0.00012207031

//! scaling factor for temperature (1.0 / 333.87) [degree]
#define YVT_X002_TEMPERATURE_SCALING_FACTOR 0.0029951776

//! shift value for temperature [degree]
#define YVT_X002_TEMPERATURE_SHIFT_VALUE 21.0

//! invalid time stamp
#define INVALID_TIMESTAMP -1.0

#define AUXILIARY_LOG_FILE_NAME "yvt_x002_auxiliary_log.csv"

static void clear_yvt_sensor_ax_data_buffer(void)
{
    int data_index = 0;
    int data_type = 0;

    for (data_index = 0; data_index < MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES; ++data_index) {
        yvt_sensor_ax_data_timestamps[data_index] = INVALID_TIMESTAMP;

        for (data_type = 0; data_type < NUMBER_OF_YVT_X002_AUXILIARY_DATA; ++data_type) {
            yvt_sensor_scaled_ax_data[data_index][data_type] = 0.0;
        }
    }

    return;
}

static void print_yvt_sensor_ax_data(const double* scaled_ax_data)
{
    printf("rotation angular velocity about x axis %3.1f [deg/msec]\n",
           scaled_ax_data[ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS]);
    printf("rotation angular velocity about y axis %3.1f [deg/msec]\n",
           scaled_ax_data[ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS]);
    printf("rotation angular velocity about z axis %3.1f [deg/msec]\n",
           scaled_ax_data[ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS]);

    printf("acceleration in x direction %3.1f [G]\n", scaled_ax_data[ACCELERATION_IN_X_DIRECTION]);
    printf("acceleration in y direction %3.1f [G]\n", scaled_ax_data[ACCELERATION_IN_Y_DIRECTION]);
    printf("acceleration in z direction %3.1f [G]\n", scaled_ax_data[ACCELERATION_IN_Z_DIRECTION]);

    printf("temperature %3.1f [degree]\n", scaled_ax_data[TEMPERATURE]);

    return;
}

static void print_valid_yvt_sensor_ax_data(void)
{
    int data_index = 0;
    int data_type = 0;
    double timestamp;
    const double *scaled_ax_data = NULL;

    for (data_index = 0; data_index < MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES; ++data_index) {

        timestamp = yvt_sensor_ax_data_timestamps[data_index];
        scaled_ax_data = yvt_sensor_scaled_ax_data[data_index];

        if (timestamp != INVALID_TIMESTAMP) {

            printf("%lf [sec]\n", timestamp / 1000.0);

            print_yvt_sensor_ax_data(scaled_ax_data);

        }

    }

    return;
}

static void write_yvt_sensor_ax_data(FILE *file_descriptor, const double *scaled_ax_data)
{
    fprintf(file_descriptor, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
            scaled_ax_data[ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS],
            scaled_ax_data[ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS],
            scaled_ax_data[ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS],
            scaled_ax_data[ACCELERATION_IN_X_DIRECTION],
            scaled_ax_data[ACCELERATION_IN_Y_DIRECTION],
            scaled_ax_data[ACCELERATION_IN_Z_DIRECTION],
            scaled_ax_data[TEMPERATURE]);

    return;
}

static void save_valid_yvt_sensor_ax_data(FILE* file_descriptor)
{
    int data_index = 0;
    int data_type = 0;
    double timestamp;
    const double *scaled_ax_data = NULL;

    for (data_index = 0; data_index < MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES; ++data_index) {

        timestamp = yvt_sensor_ax_data_timestamps[data_index];
        scaled_ax_data = yvt_sensor_scaled_ax_data[data_index];

        if (timestamp != INVALID_TIMESTAMP) {

            fprintf(file_descriptor, "%lf,", timestamp / 1000.0);
            write_yvt_sensor_ax_data(file_descriptor,scaled_ax_data);

        }

    }

    return;
}

static int is_yvt_x002_valid_auxiliary_data(const urg3d_auxiliary_data_t *raw_data)
{
    if ((raw_data->type & URG3D_GYRO_DATA) &&
        (raw_data->type & URG3D_ACCEL_DATA) &&
        (raw_data->type & URG3D_TEMPERATURE_DATA)) {

        return 1;
    }

    return 0;
}

static void set_value_unit_of_yvt_x002(const urg3d_auxiliary_record_t *raw_data,
                                       double *timestamp_msec,
                                       double yvt_scaled_data[NUMBER_OF_YVT_X002_AUXILIARY_DATA])
{
    *timestamp_msec = (double)raw_data->timestamp_ms;

    yvt_scaled_data[ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS] =
        YVT_X002_ANGULAR_VELOCITY_SCALING_FACTOR * (double)raw_data->gyro_x;
    yvt_scaled_data[ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS] =
        YVT_X002_ANGULAR_VELOCITY_SCALING_FACTOR * (double)raw_data->gyro_y;
    yvt_scaled_data[ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS] =
        YVT_X002_ANGULAR_VELOCITY_SCALING_FACTOR * (double)raw_data->gyro_z;

    yvt_scaled_data[ACCELERATION_IN_X_DIRECTION] =
        YVT_X002_ACCELERATION_SCALING_FACTOR * (double)raw_data->accel_x;
    yvt_scaled_data[ACCELERATION_IN_Y_DIRECTION] =
        YVT_X002_ACCELERATION_SCALING_FACTOR * (double)raw_data->accel_y;
    yvt_scaled_data[ACCELERATION_IN_Z_DIRECTION] =
        YVT_X002_ACCELERATION_SCALING_FACTOR * (double)raw_data->accel_z;

    yvt_scaled_data[TEMPERATURE] =
        YVT_X002_TEMPERATURE_SCALING_FACTOR * (double)raw_data->temperature + YVT_X002_TEMPERATURE_SHIFT_VALUE;

    return;
}

int main(int argc, char *argv[])
{
    int ret = 0; /* operation return */

    // sensor ip address (192.168.0.10 is default ip)
    const char* const device = "192.168.0.10";
    // sensor port number
    const long port = 10940;

    urg3d_t urg;
    urg3d_auxiliary_data_t auxiliary_data;

    int record_index = 0;
    int capturing_record_count = 0;

    int capturing_count = 0;
    int capturing_data_index = 0;

    FILE *file_descriptor;

    clear_yvt_sensor_ax_data_buffer();

    printf("open\n");
    // connect to sensor
    if((ret = urg3d_open(&urg,  device, port)) < 0) {
        printf("error urg3d_open %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("open ok\n");
    }

    printf("set blocking timeout\n");
    // set blocking function timeout = 2000[msec]
    urg3d_high_set_blocking_timeout_ms(&urg, 2000);

    // start the acquisition mode URG3D_AUXILIARY
    if((ret = urg3d_high_start_data(&urg, URG3D_AUXILIARY)) < 0) {
        printf("error urg3d_high_start_data %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    }

    // start capturing loop
    printf("start loop\n");
    while(1) {

        if (capturing_count == MAXIMUM_CAPTURING_COUNT / 2) {
            printf("%d capturing end\n", capturing_count);
        }

        if (capturing_count > MAXIMUM_CAPTURING_COUNT) {
            break;
        }

        // check receiving data
        if(urg3d_next_receive_ready(&urg)) {

            // parse ax data from receiving data
            if(urg3d_high_get_auxiliary_data(&urg, &auxiliary_data) > 0) {

                if (is_yvt_x002_valid_auxiliary_data(&auxiliary_data) > 0) {
                    ++capturing_count;

                    if (capturing_count > MAXIMUM_CAPTURING_COUNT) {
                        break;
                    }

                    capturing_record_count = auxiliary_data.record_count;
                    if (capturing_record_count > MAXIMUM_RECORD_TIMES) {
                        capturing_record_count = MAXIMUM_RECORD_TIMES;
                    }

                    for (record_index = 0; record_index < capturing_record_count; ++record_index) {
                        set_value_unit_of_yvt_x002(&auxiliary_data.records[record_index],
                                                   &yvt_sensor_ax_data_timestamps[capturing_data_index],
                                                   yvt_sensor_scaled_ax_data[capturing_data_index]);
                        ++capturing_data_index;
                    }

                }

            } else if(urg3d_low_get_binary(&urg, &header, data, &length_data) > 0) {

                // error check
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

    // stop data stream
    if((ret = urg3d_high_stop_data(&urg, URG3D_AUXILIARY)) < 0) {
        printf("error urg3d_high_stop_data %d\n", ret);
        ret = urg3d_close(&urg);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    }

    // close the connection with sensor
    if((ret = urg3d_close(&urg)) < 0) {
        printf("error urg3d_close %d\n", ret);
        #if defined(URG3D_MSC)
            getchar();
        #endif
        return -1;
    } else {
        printf("close ok\n");
    }

    printf("capturing count is %d\n", capturing_data_index);
    print_valid_yvt_sensor_ax_data();

    file_descriptor = fopen(AUXILIARY_LOG_FILE_NAME, "w");
    save_valid_yvt_sensor_ax_data(file_descriptor);
    fclose(file_descriptor);

#if defined(URG3D_MSC)
    getchar();
#endif
    return 0;
}
