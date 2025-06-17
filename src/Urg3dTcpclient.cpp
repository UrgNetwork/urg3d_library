#include "Urg3dTcpclient.h"
#include "Urg3dDetectOS.h"
#if defined(URG3D_WINDOWS_OS)
#else
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <utility>

enum {
    INVALID_DESC = -1,
};

Urg3dTcpClient::Urg3dTcpClient()
    : m_sockDesc(INVALID_DESC)
    , m_sockAddrSize(sizeof(sockaddr_in))
{
    memset((char*)&(m_serverAddr), 0, sizeof(m_sockAddrSize));
}

Urg3dTcpClient::~Urg3dTcpClient()
{
    closeConnection();
}

Urg3dTcpClient::Urg3dTcpClient(Urg3dTcpClient &&other) noexcept
    : Urg3dConnection(std::move(other))
    , m_serverAddr(other.m_serverAddr)
    , m_sockDesc(other.m_sockDesc)
    , m_sockAddrSize(other.m_sockAddrSize) {}

Urg3dTcpClient &Urg3dTcpClient::operator=(Urg3dTcpClient &&rhs) noexcept
{
    Urg3dConnection::operator=(std::move(rhs));
    m_serverAddr = rhs.m_serverAddr;
    m_sockDesc = rhs.m_sockDesc;
    m_sockAddrSize = rhs.m_sockAddrSize;

    return *this;
}

void Urg3dTcpClient::setBlockMode()
{
#if defined(URG3D_WINDOWS_OS)
    u_long flag = 0;
    ioctlsocket(m_sockDesc, FIONBIO, &flag);
#else
    int flag = 0;
    fcntl(m_sockDesc, F_SETFL, flag);
#endif
}

int32_t Urg3dTcpClient::openConnection(const char* ipAddr, long portNum)
{
    enum { CONNECT_TIMEOUT_SECOND = 2 };
    fd_set rmask, wmask;
    struct timeval tv = { CONNECT_TIMEOUT_SECOND, 0 };
#if defined(URG3D_WINDOWS_OS)
    u_long flag;
#else
    int flag;
    int sockOptval = -1;
    int sockOptvalSize = sizeof(sockOptval);
#endif
    int ret;

#if defined(URG3D_WINDOWS_OS)
    {
        static int isInitialized = 0;
        WORD wVersionRequested = 0x0202;
        WSADATA WSAData;
        int err;
        if (!isInitialized) {
            err = WSAStartup(wVersionRequested, &WSAData);
            if (err != 0) {
                return -1;
            }
            isInitialized = 1;
        }
    }

    if ((m_sockDesc = (int)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) < 0) {
        return -1;
    }
    
#else
    if ((m_sockDesc = (int)socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    
#endif

    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(portNum);

    if (ipAddr == "localhost") {
        ipAddr = "127.0.0.1";
    }

#if defined(URG3D_WINDOWS_OS)
    /* bind is not required, and port number is dynamic */
    if ((ret = inet_pton(m_serverAddr.sin_family, ipAddr, &m_serverAddr.sin_addr.S_un.S_addr)) != 1) {
        return -1;
    }
    
    // non-blocking
    flag = 1;
    ioctlsocket(m_sockDesc, FIONBIO, &flag);

    if (connect(m_sockDesc, (const struct sockaddr*)&(m_serverAddr),
                m_sockAddrSize) == SOCKET_ERROR) {
        int errorNumber = WSAGetLastError();
        if (errorNumber != WSAEWOULDBLOCK) {
            closeConnection();
            return -1;
        }

        FD_ZERO(&rmask);
        FD_SET((SOCKET)m_sockDesc, &rmask);
        wmask = rmask;

        ret = select((int)m_sockDesc + 1, &rmask, &wmask, NULL, &tv);
        if (ret == 0) {
            // timeout
            closeConnection();
            return -2;
        }
    }
    // set block mode
    setBlockMode();

#else
    /* bind is not required, and port number is dynamic */
    if ((m_serverAddr.sin_addr.s_addr = inet_addr(ipAddr)) == INADDR_NONE) {
        return -1;
    }

    // non-blocking
    flag = fcntl(m_sockDesc, F_GETFL, 0);
    fcntl(m_sockDesc, F_SETFL, flag | O_NONBLOCK);

    if (connect(m_sockDesc, (const struct sockaddr*)&(m_serverAddr),
                m_sockAddrSize) < 0) {
        if (errno != EINPROGRESS) {
            closeConnection();
            return -1;
        }

        // EINPROGRESS : starting connection but not completed
        FD_ZERO(&rmask);
        FD_SET(m_sockDesc, &rmask);
        wmask = rmask;

        ret = select(m_sockDesc + 1, &rmask, &wmask, NULL, &tv);
        if (ret <= 0) {
            // timeout
            closeConnection();
            return -2;
        }

        if (getsockopt(m_sockDesc, SOL_SOCKET, SO_ERROR, (int*)&sockOptval,
                       (socklen_t*)&sockOptvalSize) != 0) {
            // connection failed
            closeConnection();
            return -3;
        }

        if (sockOptval != 0) {
            // connection failed
            closeConnection();
            return -4;
        }

        setBlockMode();
    }
#endif

    return 0;
}

void Urg3dTcpClient::closeConnection()
{
    if (m_sockDesc != INVALID_DESC) {
#if defined(URG3D_WINDOWS_OS)
        closesocket(m_sockDesc);
#else
        close(m_sockDesc);
#endif
        m_sockDesc = INVALID_DESC;
    }
}

int32_t Urg3dTcpClient::recvData()
{
    int dataSize = m_rb->size();
    int n;
    char tmpbuf[URG3D_BUFSIZE];

    // receive with non-blocking mode.
#if defined(URG3D_WINDOWS_OS)
    u_long val = 1;
    ioctlsocket(m_sockDesc, FIONBIO, &val);
    n = recv(m_sockDesc, tmpbuf, URG3D_BUFSIZE - dataSize, 0);
    if (n < 1) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return 0;
        }
    }
#else
    n = recv(m_sockDesc, tmpbuf, URG3D_BUFSIZE - dataSize, MSG_DONTWAIT);
#endif
    if (n > 0) {
        m_rb->write(tmpbuf, n); // copy socket to my buffer
    }
#ifdef DEBUG_COMMON
    printf("receive_num = %d\n", n);
#endif
    return n;
}

int32_t Urg3dTcpClient::sendData(const char *userBuf, int reqSize)
{
#if defined(URG3D_WINDOWS_OS)
    WSABUF wsaBuf;
    DWORD dwSendLen = 0;
    DWORD dwFlags = 0;

    wsaBuf.buf = const_cast<char *>(userBuf);
    wsaBuf.len = reqSize;
    int ret = WSASend(m_sockDesc, &wsaBuf, 1, &dwSendLen, dwFlags, NULL, NULL);
    if (ret < 0) {
        return ret;
    }
    return dwSendLen;
#else
    return send(m_sockDesc, userBuf, reqSize, 0);
#endif
}

int32_t Urg3dTcpClient::error(char* errorMessage, int maxSize)
{
    (void)errorMessage;
    (void)maxSize;

    // not implemented yet.

    return -1;
}
