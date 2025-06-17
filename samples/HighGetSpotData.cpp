#include "Urg3dSensor.h"
#include <string.h>
#if defined(URG3D_WINDOWS_OS)
#include "windows.h"
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

//! ------------ Sample settings ------------
const uint8_t MOTOR_INTERLACE_COUNT = 1;
const uint8_t REM_INTERLACE_COUNT = 1;
const uint32_t MEASUREMENT_FRAME_COUNT = 100; // Number of times acquired

/* Select the spot to acquire */
const uint8_t MOTOR_FIELD_NUM = 0; // MOTOR_FIELD_NUM < MOTOR_INTERLACE_COUNT
const uint8_t REM_FIELD_NUM = 0;   // REM_FIELD_NUM < REM_INTERLACE_COUNT
const uint16_t LINE_NUM = 2;
const uint16_t SPOT_NUM = 0;
//! -----------------------------------------

URG3D_VSSP_HEADER_T header;
URG3D_MEASUREMENT_DATA_T mData;
URG3D_SENSOR_INFO_T sensorInfo;

void printSpotData(const URG3D_MEASUREMENT_DATA_T &mData,
                   uint8_t motorFieldNum, uint8_t remFieldNum, uint16_t lineNum, uint16_t spotNum)
{
    if (mData.motorFieldNumber != motorFieldNum || mData.remFieldNumber != remFieldNum || mData.lineNumber != lineNum)
    {
        return;
    }
    for (int echo = 0; echo < mData.spots[spotNum].echoCount; ++echo)
    {
        //        printf("echo = %d\n", echo);
        printf("range[m] = %f\n", mData.spots[spotNum].polar[echo].rangeM);
        //        printf("x[m] = %f, y[m] = %f, z[m] = %f\n", mData.spots[spotNum].point[echo].xm
        //            , mData.spots[spotNum].point[echo].ym
        //            , mData.spots[spotNum].point[echo].zm);
        //        printf("vAngle[rad] = %f, hAngle[rad] = %f\n", mData.spots[spotNum].polar[echo].verticalRad
        //              , mData.spots[spotNum].polar[echo].horizontalRad );
        //        printf("intensity = %d\n", mData.spots[spotNum].polar[echo].intensity);
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;                               /* operation return */
    const char *const device = "192.168.0.10"; /* device ip address */
    const long port = 10940;                   /* device port number. It is a fixed value */
    char data[URG3D_MAX_RX_LENGTH];
    int lengthData;
    string type, status;
    Urg3dSensor urg(Urg3dSensor::TCP);
    int preFrame = -1;
    FILE *fp;

    /*
     * open the connection to the sensor
     */
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

    /*
     * set blocking function timeout = 2000[ms]
     */
    urg.highSetBlockingTimeoutMs(2000);

    /*
     * initialize the urg3d session (get transform tables and sensor-specific values)
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

    /*
     * wait to finish initialize of mesurement
     */
    printf("wait initialize... (about 30-60 seconds after power-on)\n");
    if ((ret = urg.highBlockingWaitFinishedInitialize()) < 0)
    {
        printf("error urg.highBlockingWaitFinishedInitialize %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    /*
     * send interlace count for device (motor-field/frame)
     */
    if ((ret = urg.highBlockingSetMotorInterlaceCount(MOTOR_INTERLACE_COUNT)) < 0)
    {
        printf("error urg.highBlockingSetMotorInterlaceCount %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    /*
     * send interlace count for device (rem-field/frame)
     */
    if ((ret = urg.highBlockingSetRemInterlaceCount(REM_INTERLACE_COUNT)) < 0)
    {
        printf("error urg.highBlockingSetRemInterlaceCount %d\n", ret);
        ret = urg.close();
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
     *     URG3D_AUGMENTED_DISTANCE_INTENSITY
     */
    if ((ret = urg.highBlockingStartData(URG3D_DISTANCE_INTENSITY)) < 0)
    {
        printf("error urg.highBlockingStartData %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    /*
     * start main loop
     */
    printf("start loop\n");
    uint32_t fieldCount = 0;
    while (1)
    {
        /*
         * check received data
         */
        if (urg.nextReceiveReady(type, status))
        {
            /*
             * pick up the data (non-blocking)
             */
            if (urg.highGetMeasurementData(mData) > 0)
            {
                if (preFrame != mData.frameNumber)
                {
                    preFrame = mData.frameNumber;
                    if (++fieldCount > MEASUREMENT_FRAME_COUNT)
                    {
                        break;
                    }
                }
                /*
                 * add data for buffer
                 */
                if (mData.isLineDataReady)
                {
                    printSpotData(mData, MOTOR_FIELD_NUM, REM_FIELD_NUM, LINE_NUM, SPOT_NUM);
                }
            }
            else if (urg.lowGetBinary(header, data, lengthData) > 0)
            {
                /*
                 * check error data
                 */
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

    /*
     * stop the acquisition flow started earlier using the start acquisition mode.
     */
    if ((ret = urg.highBlockingStopData(URG3D_DISTANCE_INTENSITY)) < 0)
    {
        printf("error urg.highBlockingStopData %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    /*
     * close the connection to the sensor
     */
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

#if defined(URG3D_MSC)
    getchar();
#endif
    return 0;
}
