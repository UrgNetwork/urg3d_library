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

int main(int argc, char *argv[])
{
    int ret = 0;                               /* operation return */
    const char *const device = "192.168.0.10"; /* device ip address */
    const long port = 10940;                   /* device port number. It is a fixed value */
    Urg3dSensor urg(Urg3dSensor::TCP);
    string type, status;
    char data[URG3D_MAX_RX_LENGTH];
    char requestCommand[256];

    if (argc != 2)
    {
        printf("ex) %s PNG\n", argv[0]);
        printf("ex) %s VER\n", argv[0]);
        printf("ex) %s GET:_itl\n", argv[0]);
        printf("ex) %s SET:_itl=0,01\n", argv[0]);
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }

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
     * set user request command from argv[1]
     */
    sprintf(requestCommand, "%s\n", argv[1]);

    /*
     * send user request command for device
     */
    if ((ret = urg.lowRequestCommand(requestCommand)) < 0)
    {
        printf("error urg.lowRequestCommand %d\n", ret);
        ret = urg.close();
#if defined(URG3D_MSC)
        getchar();
#endif
        return -1;
    }
    else
    {
        printf("send ok -> %s\n", argv[1]);
    }

    /*
     * start main loop
     */
    printf("waiting...\n");
    while (1)
    {
        /*
         * check received data, and get binary data
         */
        if (urg.nextReceiveReady(type, status))
        {
            if (urg.lowGetBinary(header, data, lengthData) > 0)
            {
                if (strncmp(header.type, "ERR", 3) == 0 || strncmp(header.type, "_er", 3) == 0)
                {
                    printf("### receive (error) ###\n");
                    break;
                }
                printf("### receive ###\n");
                break;
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
    }
    printf("header.type = %.3s\n", header.type);
    printf("header.status = %.3s\n", header.status);
    printf("data is:\n%s", data);

    /*
     * close the connection to the sensor
     */
    if ((ret = urg.close()) < 0)
    {
        printf("error urg.close %d\n", ret);
        ret = urg.close();
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
