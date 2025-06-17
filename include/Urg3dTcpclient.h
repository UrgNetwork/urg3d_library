#ifndef URG3DTCPCLIENT_H
#define URG3DTCPCLIENT_H

#include "Urg3dConnection.h"
#include "Urg3dDetectOS.h"
#include <sys/types.h>

#if defined(URG3D_WINDOWS_OS)
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN 1
//#endif
#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

using namespace std;

class Urg3dTcpClient : public Urg3dConnection
{
public:
    Urg3dTcpClient();
    ~Urg3dTcpClient();

    Urg3dTcpClient(const Urg3dTcpClient &other) = delete;
    Urg3dTcpClient(Urg3dTcpClient &&other) noexcept;

    Urg3dTcpClient& operator=(const Urg3dTcpClient &rhs) = delete;
    Urg3dTcpClient& operator=(Urg3dTcpClient &&rhs) noexcept;

    // -- belows are MODULE INTERFACES --
    /*!
      \brief function to connect sensor

      \param[in] ipAddr IP address expressed in string, i.e. "192.168.0.1"
      \param[in] portNum port number expressed in integer, i.e. port_num = 10200

      \retval 0 succeeded.
      \retval -1 error
    */
    int32_t openConnection(const char* ipAddr, long portNum) override;

    /*!
      \brief function to disconnect sensor
    */
    void closeConnection() override;

    /*!
      \brief recieve from socket.

      \return the number of data recieved.
    */
    int32_t recvData() override;

    /*!
      \brief send to socket.

      \param[in] userBuf : data to send.
      \param[in] reqSize: data size requested to send in byte.

      \return returns the number of data sent, -1 when error.
    */
    int32_t sendData(const char* userBuf, int reqSize) override;

    //! \attention not implemented yet.
    int32_t error(char* errorMessage, int maxSize);

private:
    void setBlockMode();

    // socket
    sockaddr_in m_serverAddr;
    int32_t m_sockDesc;
    int32_t m_sockAddrSize;
};

#endif /* URG3DTCPCLIENT_H */
