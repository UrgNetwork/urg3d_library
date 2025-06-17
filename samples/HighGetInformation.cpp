#include "Urg3dSensor.h"
#include <stdio.h>
#include <stdlib.h>

URG3D_VSSP_HEADER_T header;

int main(int argc, char *argv[])
{
    int ret = 0;                               /* operation return */
    const char *const device = "192.168.0.10"; /* device ip address */
    const long port = 10940;                   /* device port number. It is a fixed value */
    Urg3dSensor urg(Urg3dSensor::TCP);
    URG3D_MEASUREMENT_DATA_T mData;
    URG3D_SENSOR_INFO_T sensorInfo;

    /*
     * open the connection to the sensor
     */
    if ((ret = urg.open(device, port)) < 0)
    {
        printf("error urg3d_open %d\n", ret);
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
     * initialize the urg3d session (for get sensor information)
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
        printf("initialized\n");
    }

    /*
     * get sensor information
     */
    if ((ret = urg.highGetSensorInfo(sensorInfo)) != 1)
    {
        printf("error not initialized\n");
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }
    else
    {
        printf("--- version ---\n");
        printf("version.vendor = %s\n", sensorInfo.version.vendor);
        printf("version.product = %s\n", sensorInfo.version.product);
        printf("version.serial = %s\n", sensorInfo.version.serial);
        printf("version.firmware = %s\n", sensorInfo.version.firmware);
        printf("version.protocol = %s\n", sensorInfo.version.protocol);
        printf("--- specification ---\n");
        printf("spec.motorInterlaceCount = %d\n", sensorInfo.spec.motorInterlaceCount);
        printf("spec.remInterlaceCount = %d\n", sensorInfo.spec.remInterlaceCount);
        printf("spec.firstLineNumber = %d\n", sensorInfo.spec.firstLineNumber);
        printf("spec.lastLineNumber = %d\n", sensorInfo.spec.lastLineNumber);
        printf("spec.lineCount = %d\n", sensorInfo.spec.lineCount);
        printf("spec.spotCount = %d\n", sensorInfo.spec.spotCount);
        printf("spec.echoCount = %d\n", sensorInfo.spec.echoCount);
        printf("spec.regionCount = %d\n", sensorInfo.spec.regionCount);
        printf("spec.isLsbResult = %d\n", sensorInfo.spec.isLsbResult);
        printf("spec.accelScale = %d\n", sensorInfo.spec.accelScale);
        printf("spec.gyroScale = %d\n", sensorInfo.spec.gyroScale);
        printf("spec.swingDir = %d\n", sensorInfo.spec.swingDir);
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
