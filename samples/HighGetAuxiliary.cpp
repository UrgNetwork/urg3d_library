#include "Urg3dSensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#include "windows.h"
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

URG3D_VSSP_HEADER_T header;
int lengthData;

enum AUXILIARY_DATA_INDEX
{
    ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS = 0,
    ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS,
    ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS,

    ACCELERATION_IN_X_DIRECTION,
    ACCELERATION_IN_Y_DIRECTION,
    ACCELERATION_IN_Z_DIRECTION,

    TEMPERATURE,

    NUMBER_OF_AUXILIARY_DATA,
};

#define MAXIMUM_RECORD_TIMES 10
#define MAXIMUM_CAPTURING_COUNT 30

double sensorAxDataTimestamps[MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES];
double sensorScaledAxData[MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES][NUMBER_OF_AUXILIARY_DATA];

//! invalid time stamp
#define INVALID_TIMESTAMP -1.0

#define AUXILIARY_LOG_FILE_NAME "auxiliary_log.csv"

static void clearSensorAxDataBuffer(void)
{
    int dataIndex = 0;
    int dataType = 0;

    for (dataIndex = 0; dataIndex < MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES; ++dataIndex)
    {
        sensorAxDataTimestamps[dataIndex] = INVALID_TIMESTAMP;

        for (dataType = 0; dataType < NUMBER_OF_AUXILIARY_DATA; ++dataType)
        {
            sensorScaledAxData[dataIndex][dataType] = 0.0;
        }
    }

    return;
}

static void printSensorAxData(const double *scaledAxData)
{
    printf("rotation angular velocity about x axis %3.1f [deg/sec]\n",
           scaledAxData[ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS]);
    printf("rotation angular velocity about y axis %3.1f [deg/sec]\n",
           scaledAxData[ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS]);
    printf("rotation angular velocity about z axis %3.1f [deg/sec]\n",
           scaledAxData[ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS]);

    printf("acceleration in x direction %3.1f [G]\n", scaledAxData[ACCELERATION_IN_X_DIRECTION]);
    printf("acceleration in y direction %3.1f [G]\n", scaledAxData[ACCELERATION_IN_Y_DIRECTION]);
    printf("acceleration in z direction %3.1f [G]\n", scaledAxData[ACCELERATION_IN_Z_DIRECTION]);

    printf("temperature %3.1f [degree]\n", scaledAxData[TEMPERATURE]);

    return;
}

static void printValidSensorAxData(void)
{
    int dataIndex = 0;
    int dataType = 0;
    double timestamp;
    const double *scaledAxData = NULL;

    for (dataIndex = 0; dataIndex < MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES; ++dataIndex)
    {
        timestamp = sensorAxDataTimestamps[dataIndex];
        scaledAxData = sensorScaledAxData[dataIndex];

        if (timestamp != INVALID_TIMESTAMP)
        {
            printf("%lf [sec]\n", timestamp / 1000.0);
            printSensorAxData(scaledAxData);
        }
    }

    return;
}

static void writeSensorAxData(FILE *fileDescriptor, const double *scaledAxData)
{
    fprintf(fileDescriptor, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
            scaledAxData[ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS],
            scaledAxData[ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS],
            scaledAxData[ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS],
            scaledAxData[ACCELERATION_IN_X_DIRECTION],
            scaledAxData[ACCELERATION_IN_Y_DIRECTION],
            scaledAxData[ACCELERATION_IN_Z_DIRECTION],
            scaledAxData[TEMPERATURE]);

    return;
}

static void saveValidSensorAxData(FILE *fileDescriptor)
{
    int dataIndex = 0;
    double timestamp;
    const double *scaledAxData = NULL;

    fprintf(fileDescriptor, "timestamp,gyroX,gyroY,gyroZ,accX,accY,accZ,temperature\n");

    for (dataIndex = 0; dataIndex < MAXIMUM_CAPTURING_COUNT * MAXIMUM_RECORD_TIMES; ++dataIndex)
    {

        timestamp = sensorAxDataTimestamps[dataIndex];
        scaledAxData = sensorScaledAxData[dataIndex];

        if (timestamp != INVALID_TIMESTAMP)
        {

            fprintf(fileDescriptor, "%lf,", timestamp / 1000.0);
            writeSensorAxData(fileDescriptor, scaledAxData);
        }
    }

    return;
}

static int isValidAuxiliaryData(const URG3D_AUXILIARY_DATA_T &rawData)
{
    if ((rawData.type & URG3D_GYRO_DATA) &&
        (rawData.type & URG3D_ACCEL_DATA) &&
        (rawData.type & URG3D_TEMPERATURE_DATA))
    {

        return 1;
    }

    return 0;
}

static void setValueUnitOfSensor(const URG3D_AUXILIARY_RECORD_T &rawData,
                                 double &timestampMsec,
                                 double scaledData[NUMBER_OF_AUXILIARY_DATA],
                                 URG3D_SENSOR_INFO_T &info)
{
    //! scaling factor for angular velocity (gyro scale / 32767) [deg/sec]
    const double ANGULAR_VELOCITY_SCALING_FACTOR = ((double)info.spec.gyroScale / 32767.0);
    //! scaling factor for acceleration (accel scale / 32767) [G]
    const double ACCELERATION_SCALING_FACTOR = ((double)info.spec.accelScale / 32767.0);
    //! scaling factor for temperature (1.0 / 333.87) [degree Celsius]
    const double TEMPERATURE_SCALING_FACTOR = 0.0029951776;
    //! shift value for temperature [degree Celsius]
    const double TEMPERATURE_SHIFT_VALUE = 21.0;

    timestampMsec = (double)rawData.timestampMs;

    scaledData[ROTATION_ANGULAR_VELOCITY_ABOUT_X_AXIS] =
        ANGULAR_VELOCITY_SCALING_FACTOR * (double)rawData.gyroX;
    scaledData[ROTATION_ANGULAR_VELOCITY_ABOUT_Y_AXIS] =
        ANGULAR_VELOCITY_SCALING_FACTOR * (double)rawData.gyroY;
    scaledData[ROTATION_ANGULAR_VELOCITY_ABOUT_Z_AXIS] =
        ANGULAR_VELOCITY_SCALING_FACTOR * (double)rawData.gyroZ;

    scaledData[ACCELERATION_IN_X_DIRECTION] =
        ACCELERATION_SCALING_FACTOR * (double)rawData.accelX;
    scaledData[ACCELERATION_IN_Y_DIRECTION] =
        ACCELERATION_SCALING_FACTOR * (double)rawData.accelY;
    scaledData[ACCELERATION_IN_Z_DIRECTION] =
        ACCELERATION_SCALING_FACTOR * (double)rawData.accelZ;

    scaledData[TEMPERATURE] =
        TEMPERATURE_SCALING_FACTOR * (double)rawData.temperature + TEMPERATURE_SHIFT_VALUE;

    return;
}

int main(int argc, char *argv[])
{
    int ret = 0; /* operation return */

    // sensor ip address (192.168.0.10 is default ip)
    const char *const device = "192.168.0.10";
    // sensor port number
    const long port = 10940;

    Urg3dSensor urg(Urg3dSensor::TCP);
    URG3D_AUXILIARY_DATA_T auxiliaryData;
    URG3D_MEASUREMENT_DATA_T mData;
    URG3D_SENSOR_INFO_T sensorInfo;
    string type, status;
    char data[URG3D_MAX_RX_LENGTH];

    int recordIndex = 0;
    int capturingRecordCount = 0;

    int capturingCount = 0;
    int capturingDataIndex = 0;

    FILE *fileDescriptor;

    clearSensorAxDataBuffer();

    printf("open\n");
    // connect to sensor
    if ((ret = urg.open(device, port)) < 0)
    {
        printf("error urg.open %d\n", ret);
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }
    else
    {
        printf("open ok\n");
    }

    printf("set blocking timeout\n");
    // set blocking function timeout = 2000[msec]
    urg.highSetBlockingTimeoutMs(2000);

    // start the acquisition mode URG3D_AUXILIARY
    if ((ret = urg.highBlockingStartData(URG3D_AUXILIARY)) < 0)
    {
        printf("error urg.highBlockingStartData %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    /*
     * initialize the urg3d session (for get sensor-specific values)
     */
    if ((ret = urg.highBlockingInit(mData)) < 0)
    {
        printf("error urg.highBlockingInit %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }
    else
    {
        urg.highGetSensorInfo(sensorInfo);
        printf("initialized\n");
    }

    // start capturing loop
    printf("start loop\n");
    while (1)
    {

        if (capturingCount == MAXIMUM_CAPTURING_COUNT / 2)
        {
            printf("%d capturing end\n", capturingCount);
        }

        if (capturingCount > MAXIMUM_CAPTURING_COUNT)
        {
            break;
        }

        // check receiving data
        if (urg.nextReceiveReady(type, status))
        {

            // parse ax data from receiving data
            if (urg.highGetAuxiliaryData(auxiliaryData) > 0)
            {

                if (isValidAuxiliaryData(auxiliaryData) > 0)
                {
                    ++capturingCount;

                    if (capturingCount > MAXIMUM_CAPTURING_COUNT)
                    {
                        break;
                    }

                    capturingRecordCount = auxiliaryData.recordCount;
                    if (capturingRecordCount > MAXIMUM_RECORD_TIMES)
                    {
                        capturingRecordCount = MAXIMUM_RECORD_TIMES;
                    }

                    for (recordIndex = 0; recordIndex < capturingRecordCount; ++recordIndex)
                    {
                        setValueUnitOfSensor(auxiliaryData.records[recordIndex],
                                             sensorAxDataTimestamps[capturingDataIndex],
                                             sensorScaledAxData[capturingDataIndex],
                                             sensorInfo);
                        ++capturingDataIndex;
                    }
                }
            }
            else if (urg.lowGetBinary(header, data, lengthData) > 0)
            {

                // error check
                if (strncmp(header.type, "ERR", 3) == 0 || strncmp(header.type, "_er", 3) == 0)
                {
                    printf("error %c%c%c %s", header.status[0], header.status[1], header.status[2], data);
                    if (header.status[0] != '0')
                    {
                        break;
                    }
                }
            }
        }
        else
        {
#ifdef URG3D_WINDOWS_OS
            Sleep(10);
#else
            usleep(10000);
#endif
        }
    }

    // stop data stream
    if ((ret = urg.highBlockingStopData(URG3D_AUXILIARY)) < 0)
    {
        printf("error urg.highBlockingStopData %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    // close the connection with sensor
    if ((ret = urg.close()) < 0)
    {
        printf("error urg.close %d\n", ret);
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }
    else
    {
        printf("close ok\n");
    }

    printf("capturing count is %d\n", capturingDataIndex);
    printValidSensorAxData();

    fileDescriptor = fopen(AUXILIARY_LOG_FILE_NAME, "w");
    saveValidSensorAxData(fileDescriptor);
    fclose(fileDescriptor);

    printf("\nDone!\n");

#if defined(URG3D_MSC)
    getchar();
#endif
    return 0;
}
