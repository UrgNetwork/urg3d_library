#include "Urg3dConnection.h"

Urg3dConnection::Urg3dConnection()
    : m_rb(new Urg3dRingBuffer(URG3D_RB_BITSHIFT)) {}

Urg3dConnection::~Urg3dConnection()
{
    delete m_rb;
}

Urg3dConnection::Urg3dConnection(Urg3dConnection &&other)
    : m_rb(other.m_rb)
{
    other.m_rb = nullptr;
}

Urg3dConnection &Urg3dConnection::operator=(Urg3dConnection &&rhs) noexcept
{
    delete m_rb;

    m_rb = rhs.m_rb;
    rhs.m_rb = nullptr;

    return *this;
}

void Urg3dConnection::clearRing()
{
    m_rb->clear();
}
