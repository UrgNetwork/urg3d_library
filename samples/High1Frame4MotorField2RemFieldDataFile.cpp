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
URG3D_MEASUREMENT_DATA_T mData;
URG3D_SENSOR_INFO_T sensorInfo;
int lengthData;
int bufferLength = 0;
char buffer[2 * 1024 * 1024]; // 2MB

void toBuffer(const URG3D_MEASUREMENT_DATA_T &mData)
{
    int spot, echo;
    bufferLength += sprintf(buffer + bufferLength, "#x_m,#y_m,#z_m,#range_m,#vertical_rad,#horizontal_rad,#intensity,#spot(0-%d),#echo(0-max%d),#->frame:motor_field:rem_field:line->,%d,%d,%d,%d\n", sensorInfo.spec.spotCount - 1, sensorInfo.spec.echoCount - 1, mData.frameNumber, mData.motorFieldNumber, mData.remFieldNumber, mData.lineNumber);
    for (spot = 0; spot < mData.spotCount; ++spot)
    {
        for (echo = 0; echo < mData.spots[spot].echoCount; ++echo)
        {
            bufferLength += sprintf(buffer + bufferLength, "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%d\n", mData.spots[spot].point[echo].xm, mData.spots[spot].point[echo].ym, mData.spots[spot].point[echo].zm, mData.spots[spot].polar[echo].rangeM, mData.spots[spot].polar[echo].verticalRad, mData.spots[spot].polar[echo].horizontalRad, mData.spots[spot].polar[echo].intensity, spot, echo);
        }
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;                               /* operation return */
    const char *const device = "192.168.0.10"; /* device ip address */
    const long port = 10940;                   /* device port number. It is a fixed value */
    char data[URG3D_MAX_RX_LENGTH];
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
     * send interlace count for device (2 motor-field/rem-field)
     */
    if ((ret = urg.highBlockingSetMotorInterlaceCount(2)) < 0)
    {
        printf("error urg.highBlockingSetMotorInterlaceCount %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

    /*
     * send interlace count for device (4 rem-field/frame)
     */
    if ((ret = urg.highBlockingSetRemInterlaceCount(4)) < 0)
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
                /*
                 * wait for first data
                 */
                if (preFrame == -1)
                {
                    /*
                     * check line and field number
                     */
                    if (mData.lineNumber == sensorInfo.spec.firstLineNumber && mData.motorFieldNumber == 0 && mData.remFieldNumber == 0)
                    {
                        preFrame = mData.frameNumber;
                    }
                }
                /*
                 * start frame data
                 */
                if (preFrame != -1)
                {
                    /*
                     * if frame number is changed, break the loop
                     */
                    if (preFrame != mData.frameNumber)
                    {
                        break;
                    }
                    /*
                     * add data for buffer
                     */
                    if (mData.isLineDataReady)
                    {
                        toBuffer(mData);
                    }
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
     * save csv format file
     */
    fp = fopen("output.csv", "w");
    int wroteSize = fwrite(buffer, sizeof(char), bufferLength, fp);
    fclose(fp);
    printf("save output.csv\n");

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
