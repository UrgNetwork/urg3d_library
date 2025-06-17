#ifndef URG3DRINGBUFFER_H
#define URG3DRINGBUFFER_H

#include <stdint.h>

class Urg3dRingBuffer
{
public:
    explicit Urg3dRingBuffer(uint8_t shiftSize);
    ~Urg3dRingBuffer();

    Urg3dRingBuffer(const Urg3dRingBuffer &other);
    Urg3dRingBuffer(Urg3dRingBuffer &&other) noexcept;

    Urg3dRingBuffer& operator=(const Urg3dRingBuffer &rhs);
    Urg3dRingBuffer& operator=(Urg3dRingBuffer &&rhs) noexcept;

    /*!
      \brief function to clear ring buffer
    */
    void clear();

    /*!
      \brief function to return amount of stored data
    */
    uint32_t size();

    /*!
      \brief function to return maximum amount of stored data
    */
    uint32_t capacity();

    /*!
      \brief function to store data in ring buffer

      \param[in] data data to store
      \param[in] size size of data

      \return amount of stored data
    */
    uint32_t write(const char* data, uint32_t pushSize);

    /*!
      \brief function to read data from ring buffer

      \param[out] buffer data
      \param[in] popSize size of data to be poped

      \return amount of read data
    */
    uint32_t read(char* buffer, uint32_t popSize);

    /*!
      \brief function to read data from ring buffer without changing the head position

      \param[out] buffer data
      \param[in] popSize size of data to be poped

      \return amount of read data
    */
    uint32_t peek(char* buffer, uint32_t popSize);

private:
    char* mp_buffer;       //!< pointer
    uint32_t m_bufferSize;  //!< buffer size
    uint32_t m_dataLength;  //!< data length
    uint32_t m_first;       //!< start point of buffer
    uint32_t m_last;        //!< end point of buffer

    const uint32_t mc_mask; //!< wrap-around mask
};


#endif /* URG3DRINGBUFFER_H */
