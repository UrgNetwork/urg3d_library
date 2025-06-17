#include "Urg3dRingBuffer.h"

#include <string.h>

Urg3dRingBuffer::Urg3dRingBuffer(uint8_t shiftSize)
    : mp_buffer(new char[1 << shiftSize])
    , m_bufferSize(1 << shiftSize)
    , m_dataLength(0)
    , m_first(0)
    , m_last(0)
    , mc_mask(m_bufferSize - 1)
{
}

Urg3dRingBuffer::~Urg3dRingBuffer()
{
    delete[] mp_buffer;
}

Urg3dRingBuffer::Urg3dRingBuffer(const Urg3dRingBuffer &other)
    : m_bufferSize(other.m_bufferSize)
    , m_dataLength(other.m_dataLength)
    , m_first(other.m_first)
    , m_last(other.m_last)
    , mc_mask(other.m_bufferSize - 1)
{
    mp_buffer = new char[m_bufferSize];
    memcpy(mp_buffer, other.mp_buffer, m_bufferSize);
}

Urg3dRingBuffer::Urg3dRingBuffer(Urg3dRingBuffer &&other) noexcept
    : mp_buffer(other.mp_buffer)
    , m_bufferSize(other.m_bufferSize)
    , m_dataLength(other.m_dataLength)
    , m_first(other.m_first)
    , m_last(other.m_last)
    , mc_mask(other.mc_mask)
{
    other.mp_buffer = nullptr;
}

Urg3dRingBuffer &Urg3dRingBuffer::operator=(const Urg3dRingBuffer &rhs)
{
    delete[] mp_buffer;

    m_bufferSize = rhs.m_bufferSize;
    m_dataLength = rhs.m_dataLength;
    m_first = rhs.m_first;
    m_last = rhs.m_last;
    mp_buffer = new char[m_bufferSize];
    memcpy(mp_buffer, rhs.mp_buffer, m_bufferSize);

    return *this;
}

Urg3dRingBuffer &Urg3dRingBuffer::operator=(Urg3dRingBuffer &&rhs) noexcept
{
    delete[] mp_buffer;

    mp_buffer = rhs.mp_buffer;
    m_bufferSize = rhs.m_bufferSize;
    m_dataLength = rhs.m_dataLength;
    m_first = rhs.m_first;
    m_last = rhs.m_last;

    rhs.mp_buffer = nullptr;

    return *this;

}

void Urg3dRingBuffer::clear()
{
    m_first = 0;
    m_last = 0;
}

uint32_t Urg3dRingBuffer::size()
{
    if (m_last >= m_first) {
        return m_last - m_first;
    } else {
        return m_last - m_first + m_bufferSize;
    }
}

uint32_t Urg3dRingBuffer::capacity()
{
    return m_bufferSize - 1;
}

uint32_t Urg3dRingBuffer::write(const char* data, uint32_t pushSize)
{
    uint32_t freeSize = m_bufferSize - m_dataLength;

    if (freeSize > pushSize) {
        // storing data
        if (m_first <= m_last) {
            // set from last to end point (length : buffer_size)
            uint32_t leftSize = 0;
            uint32_t toEnd = m_bufferSize - m_last;
            uint32_t copySize = (toEnd > pushSize) ? pushSize : toEnd;

            memcpy(&mp_buffer[m_last], data, copySize);
            m_last += copySize;
            m_last &= mc_mask;

            if (pushSize > copySize) {
                leftSize = pushSize - copySize;
                // set from 0 to before first
                memcpy(mp_buffer, &data[copySize], leftSize);
                m_last = leftSize;
            }
        } else {
            // set from last to before first
            memcpy(&mp_buffer[m_last], data, pushSize);
            m_last += pushSize;
        }
        return pushSize;
    }
    return 0;
}

uint32_t Urg3dRingBuffer::read(char* buffer, uint32_t popSize)
{
    // receive data
    uint32_t nowSize = size();

    if (nowSize >= popSize) {
        if (m_first <= m_last) {
            memcpy(buffer, &mp_buffer[m_first], popSize);
            m_first += popSize;
        } else {
            // set from first to end point (length : buffer_size)
            uint32_t leftSize = 0;
            uint32_t toEnd = m_bufferSize - m_first;
            uint32_t copySize = (toEnd > popSize) ? popSize : toEnd;
            memcpy(buffer, &mp_buffer[m_first], copySize);

            m_first += copySize;
            m_first &= mc_mask;

            if (popSize > copySize) {
                leftSize = popSize - copySize;
                // set from 0 to before last
                memcpy(&buffer[copySize], mp_buffer, leftSize);

                m_first = leftSize;
            }
        }
        return popSize;
    }
    return 0;
}

uint32_t Urg3dRingBuffer::peek(char* buffer, uint32_t popSize)
{
    uint32_t nowSize = size();

    if (nowSize >= popSize) {
        if (m_first <= m_last) {
            memcpy(buffer, &mp_buffer[m_first], popSize);
        } else {
            uint32_t leftSize = 0;
            uint32_t toEnd = m_bufferSize - m_first;
            uint32_t copySize = (toEnd > popSize) ? popSize : toEnd;
            memcpy(buffer, &mp_buffer[m_first], copySize);

            if (popSize > copySize) {
                leftSize = popSize - copySize;
                memcpy(&buffer[copySize], mp_buffer, leftSize);
            }
        }
        return popSize;
    }
    return 0;
}

