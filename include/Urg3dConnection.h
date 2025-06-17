#ifndef URG3DCONNECTION_H
#define URG3DCONNECTION_H

#include <stdint.h>
#include "Urg3dErrno.h"
#include "Urg3dRingBuffer.h"

const uint8_t URG3D_MAX_RX_BUFFER_BIT = 17; //This is recommend
//const uint8_t URG3D_MAX_RX_BUFFER_BIT = 12; //This is minimize 4096 byte

// -- NOT INTERFACE, for internal use only --
// For Urg3dRingBuffer.h
// The size of buffer must be specified by the power of 2
// i.e. ring buffer size = two to the URG3D_RB_BITSHIFT-th power.
enum URG3D_BUFFER_SIZE {
    URG3D_RB_BITSHIFT = URG3D_MAX_RX_BUFFER_BIT,
    URG3D_RB_SIZE = 1 << URG3D_RB_BITSHIFT,

    // caution ! available buffer size is less than the
    // size given to the ring buffer(URG3D_RB_SIZE).
    URG3D_BUFSIZE = URG3D_RB_SIZE - 1,
};

// -- end of NON INTERFACE definitions --

class Urg3dConnection
{
public:
    Urg3dConnection();
    virtual ~Urg3dConnection();

    Urg3dConnection(const Urg3dConnection &other) = delete;
    Urg3dConnection(Urg3dConnection &&other);

    Urg3dConnection& operator=(const Urg3dConnection &rhs) = delete;
    Urg3dConnection& operator=(Urg3dConnection &&rhs) noexcept;

    /*!
      \brief function to connect sensor

      \param[in] ip   : ip address
      \param[in] port : port number

      \retval 0 success
      \retval <0 error
    */
    virtual int32_t openConnection(const char* ip, long port) = 0;

    /*!
      \brief function to disconnect sensor
    */
    virtual void closeConnection() = 0;

    /*!
      \brief receive data via communication.

      \return the number of data recieved.
    */
    virtual int32_t recvData() = 0;

    /*!
      \brief send data via communication.

      \param[in] userBuf : data to send.
      \param[in] reqSize: data size requested to send in byte.

      \return returns the number of data sent, -1 when error.
    */
    virtual int32_t sendData(const char* userBuf, int reqSize) = 0;

    /*!
      \brief function to get ring buffer pointer
    */
    Urg3dRingBuffer* ringBuffer() { return m_rb; }

    /*!
      \brief function to clear ring buffer
    */
    void clearRing();

protected:
    Urg3dRingBuffer *m_rb;
};


#endif
