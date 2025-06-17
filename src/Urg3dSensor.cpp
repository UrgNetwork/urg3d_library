#define _CRT_SECURE_NO_WARNINGS

#include "Urg3dSensor.h"
#include "Urg3dTcpclient.h"
#include "Urg3dTicks.h"
#if defined(URG3D_WINDOWS_OS)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

//#define DEBUG_COMMON
//#define DEBUG_LOW
//#define DEBUG_HIGH

Urg3dSensor::Urg3dSensor(URG3D_CONNECTION_TYPE type)
    : m_connectionType(type)
    , m_isInit(URG3D_FALSE)
    , m_isActive(URG3D_FALSE)
    , m_lastErrno(URG3D_NOT_CONNECTED)
    , m_blockingTimeoutMs(1000)
{
    switch (m_connectionType) {
    // Structured to accommodate different protocols if required
    case TCP :
    default:
      m_connection = std::unique_ptr<Urg3dTcpClient>(new Urg3dTcpClient());
      break;
    }
}

int32_t Urg3dSensor::open(const char* ip, int port)
{
    int ret = 0;

    //connect
    if ((ret = m_connection->openConnection(ip, port)) < 0) {
        m_lastErrno = URG3D_ETHERNET_OPEN_ERROR;
        return m_lastErrno;
    }

    // stop the _ro/_ri/_ax data acquisition. (for YVT)
    for (int type = URG3D_DISTANCE; type <= URG3D_AUXILIARY; ++type) {
        if ((ret = highBlockingStopData((URG3D_MEASUREMENT_TYPE)type)) < 0) {
            close();
            m_lastErrno = URG3D_INVALID_RESPONSE;
            return m_lastErrno;
        }
    }

    m_isActive = URG3D_TRUE;
    return 0;
}

int32_t Urg3dSensor::close()
{
    lowPurge();
    m_connection->closeConnection();
    m_isActive = URG3D_FALSE;
    m_isInit = URG3D_FALSE;
    return 0;
}

int8_t Urg3dSensor::isActive()
{
    return m_isActive;
}

int32_t Urg3dSensor::nextReceiveReady(string& type, string& status)
{
    char vsspTmp[4] = { 0 };
    type.resize(URG3D_TYPE_LENGTH, '\0');
    status.resize(URG3D_STATUS_LENGTH, '\0');

    m_connection->recvData();
    auto *rb = m_connection->ringBuffer();

    if (m_nextPacket.nextHeaderReady == 0) {
        //search VSSP
        while (1) {
            if (rb->size() <= 4) {
                return 0;
            }
            rb->peek(vsspTmp, 4);
            if (strncmp(vsspTmp, "VSSP", 4) == 0) {
                break;
            } else {
                rb->read(vsspTmp, 1);
            }
        }

        //check VSSP header length
        if (rb->size() < URG3D_VSSP_HEADER_LENGTH) {
            return 0;
        } else {
            //pre read VSSP header
            rb->peek((char*)(&m_nextPacket.nextHeader), URG3D_VSSP_HEADER_LENGTH);
            m_nextPacket.nextHeaderReady = 1;
        }
    }

    if (m_nextPacket.nextHeaderReady == 1) {
        strncpy(const_cast<char*>(type.c_str()), &m_nextPacket.nextHeader[4], URG3D_TYPE_LENGTH);
        strncpy(const_cast<char*>(status.c_str()), &m_nextPacket.nextHeader[8], URG3D_STATUS_LENGTH);
    }

    if (m_nextPacket.nextDataReady == 0) {
        //check data length
        if (rb->size() < ((*(uint16_t*)(m_nextPacket.nextHeader + 14)))) {
            return 0;
        } else {
            m_nextPacket.nextDataReady = 1;
        }
    }
    return m_nextPacket.nextHeaderReady && m_nextPacket.nextDataReady;
}

void Urg3dSensor::highSetBlockingTimeoutMs(uint32_t timeoutMs)
{
    m_blockingTimeoutMs = timeoutMs;
}

int32_t Urg3dSensor::highBlockingInit(URG3D_MEASUREMENT_DATA_T& mData)
{
    int32_t ret = 0;
    uint8_t intl = 0;

    // get sensor version
    ret = highBlockingGetSensorVersion(m_info.version);
    if (ret < 0) {
        return ret;
    }
    int32_t majorVersion = 0;
    int32_t minorVersion = 0;
    sscanf(m_info.version.protocol, "%i.%i\0", &majorVersion, &minorVersion);

    // get sensor-specific values
    if (majorVersion == 2) {
        if (minorVersion >= 3) {
            ret = highBlockingGetSensorSpecification(m_info.spec);
            if (ret < 0) {
                return ret;
            }
        } else {
            setConstantParameter(m_info.spec);
        }
    } else {
        setConstantParameter(m_info.spec);
    }
    // resize member
    resizeMembers(mData);

    for (int i = 0; i < m_info.spec.remInterlaceCount; ++i) {
        m_tables.spotRemAngleLoaded[i] = 0;
    }

    // get angle table
    ret = highBlockingGetMotorInterlaceCount(intl);
    if (ret < 0) {
        return ret;
    }

    if (m_info.spec.spotCount > URG3D_VSSP_SPOTS_IN_GROUP) {
        ret = highBlockingGetMotorAngleTableMulti(m_info.spec.spotCount);
    } else {
        ret = highBlockingGetMotorAngleTable();
    }
    if (ret < 0) {
        return ret;
    }

    ret = highBlockingGetRemInterlaceCount(intl);
    if (ret < 0) {
        return ret;
    }

    if (m_info.spec.spotCount > URG3D_VSSP_SPOTS_IN_GROUP) {
        ret = highBlockingGetRemAngleTableMulti(intl, m_info.spec.spotCount);
    } else {
        ret = highBlockingGetRemAngleTable(intl);
    }
    if (ret < 0) {
        return ret;
    }

    m_isInit = URG3D_TRUE;

    return 1;
}

int32_t Urg3dSensor::highBlockingWaitFinishedInitialize()
{
    int ret = 0;
    int64_t startTimeMs = Urg3dTicks::Urg3dTicksMs();
    int64_t timeoutMs = 60 * 1000; // Timeout greater than sensor startup time
    URG3D_VSSP_HEADER_T header;
    char* data_p, data[URG3D_MAX_RX_LENGTH] = { 0 };
    
    while (1) {
        int ro = 0, ri = 0, ax = 0;
        ret = highBlockingCommon(header, data, "GET:stat\n");
        if (ret < 0) {
            return ret;
        }
        data_p = data;
        while (data_p[0] != '\0') {
            if (strncmp(data_p, "_ro=", 4) == 0) {
                if (strncmp(data_p + 4, "000", 3) == 0) { ro = 1; }
                if (strncmp(data_p + 4, "099", 3) == 0) { ro = 99; }
                if (ro == 0) { ro = -1; }
            }
            if (strncmp(data_p, "_ri=", 4) == 0) {
                if (strncmp(data_p + 4, "000", 3) == 0) { ri = 1; }
                if (strncmp(data_p + 4, "099", 3) == 0) { ri = 99; }
                if (ro == 0) { ro = -1; }
            }
            if (strncmp(data_p, "_ax=", 4) == 0) {
                if (strncmp(data_p + 4, "000", 3) == 0) { ax = 1; }
                if (strncmp(data_p + 4, "099", 3) == 0) { ax = 99; }
                if (ax == 0) { ax = -1; }
            }
            while (data_p[0] != '\0' && data_p[0] != '\n') {
                ++data_p;
            }
            if (data_p[0] == '\n') {
                ++data_p;
            }
        }
        
        if (Urg3dTicks::Urg3dTicksMs() - startTimeMs >= timeoutMs) {
            return -2;
        }
#ifdef DEBUG_HIGH

#endif
        if (ro == -1 || ri == -1 || ax == -1) { return -1; }
        if (ro == 1 && ri == 1 && ax == 1) { break; }
    }

    return 1;
}

int32_t Urg3dSensor::highBlockingGetSensorSpecification(URG3D_SENSOR_SPEC_T& spec)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int32_t ret = 0;

    ret = highBlockingCommon(header, data, "GET:spec.motorInterlaceCount\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.motorInterlaceCount\n%hhu\n", &spec.motorInterlaceCount);

    ret = highBlockingCommon(header, data, "GET:spec.remInterlaceCount\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.remInterlaceCount\n%hhu\n", &spec.remInterlaceCount);

    ret = highBlockingCommon(header, data, "GET:spec.firstLineNumber\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.firstLineNumber\n%hu\n", &spec.firstLineNumber);

    ret = highBlockingCommon(header, data, "GET:spec.lastLineNumber\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.lastLineNumber\n%hu\n", &spec.lastLineNumber);

    ret = highBlockingCommon(header, data, "GET:spec.spotCount\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.spotCount\n%hu\n", &spec.spotCount);

    ret = highBlockingCommon(header, data, "GET:spec.echoCount\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.echoCount\n%hhu\n", &spec.echoCount);

    ret = highBlockingCommon(header, data, "GET:spec.regionCount\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.regionCount\n%hhu\n", &spec.regionCount);

    ret = highBlockingCommon(header, data, "GET:spec.isLsbResult\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.isLsbResult\n%hhu\n", &spec.isLsbResult);

    ret = highBlockingCommon(header, data, "GET:spec.accelScale\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.accelScale\n%hu\n", &spec.accelScale);

    ret = highBlockingCommon(header, data, "GET:spec.gyroScale\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:spec.gyroScale\n%hu\n", &spec.gyroScale);

    ret = highBlockingCommon(header, data, "GET:swdr\n");
    if (ret < 0) {
        return ret;
    }
    sscanf(data, "GET:swdr\n%hhu\n", &spec.swingDir);

    if (spec.lastLineNumber >= spec.firstLineNumber) {
        spec.lineCount = spec.lastLineNumber - spec.firstLineNumber + 1;
    }

#ifdef DEBUG_HIGH
    printf("spec.motorInterlaceCount = %hhu\n", spec.motorInterlaceCount);
    printf("spec.remInterlaceCount = %hhu\n", spec.remInterlaceCount);
    printf("spec.firstLineNumber = %hu\n", spec.firstLineNumber);
    printf("spec.lastLineNumber = %hu\n", spec.lastLineNumber);
    printf("spec.lineCount = %hu\n", spec.lineCount);
    printf("spec.spotCount = %hu\n", spec.spotCount);
    printf("spec.echoCount = %hhu\n", spec.echoCount);
    printf("spec.regionCount = %hhu\n", spec.regionCount);
    printf("spec.isLsbResult = %hhu\n", spec.isLsbResult);
    printf("spec.accelScale = %hu\n", spec.accelScale);
    printf("spec.gyroScale = %hu\n", spec.gyroScale);
    printf("spec.swingDir = %hhu\n", spec.swingDir);
    printf("\n");
#endif

    return 1;
}

int32_t Urg3dSensor::highBlockingGetSensorVersion(URG3D_SENSOR_VERSION_T& version)
{
    URG3D_VSSP_HEADER_T header;
    char* data_p, data[URG3D_MAX_RX_LENGTH] = { 0 };
    int ret = 0;

    ret = highBlockingCommon(header, data, "VER\n", "vend");
    if (ret < 0) {
        return ret;
    }

    version.vendor[0] = '\0';
    version.product[0] = '\0';
    version.serial[0] = '\0';
    version.firmware[0] = '\0';
    version.protocol[0] = '\0';
    data_p = data;
    while (data_p[0] != '\0') {
        if (strncmp(data_p, "vend:", 5) == 0) {
            sscanf(data_p, "vend:%[^\n]\n", version.vendor);
        }
        if (strncmp(data_p, "prod:", 5) == 0) {
            sscanf(data_p, "prod:%[^\n]\n", version.product);
        }
        if (strncmp(data_p, "seri:", 5) == 0) {
            sscanf(data_p, "seri:%[^\n]\n", version.serial);
        }
        if (strncmp(data_p, "firm:", 5) == 0) {
            sscanf(data_p, "firm:%[^\n]\n", version.firmware);
        }
        if (strncmp(data_p, "prot:", 5) == 0) {
            sscanf(data_p, "prot:%[^\n]\n", version.protocol);
        }
        while (data_p[0] != '\0' && data_p[0] != '\n') {
            ++data_p;
        }
        if (data_p[0] == '\n') {
            ++data_p;
        }
    }

#ifdef DEBUG_HIGH
    printf("version.vendor = %s\n", version.vendor);
    printf("version.product = %s\n", version.product);
    printf("version.serial = %s\n", version.serial);
    printf("version.firmware = %s\n", version.firmware);
    printf("version.protocol = %s\n", version.protocol);
    printf("\n");
#endif

    return 1;
}

int32_t Urg3dSensor::highBlockingGetMotorInterlaceCount(uint8_t& count)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int32_t ret = 0;

    ret = highBlockingCommon(header, data, "GET:_itl\n");
    if (ret < 0) {
        return ret;
    }

    sscanf(data, "GET:_itl\n0,%hhu", &count);

#ifdef DEBUG_HIGH
    printf("sizeof(*count) = %u\n", sizeof(count));
    printf("*count = %hhu\n", count);
    printf("\n");
#endif
    return 1;
}

int32_t Urg3dSensor::highBlockingGetRemInterlaceCount(uint8_t& count)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int32_t ret = 0;

    ret = highBlockingCommon(header, data, "GET:_itv\n");
    if (ret < 0) {
        return ret;
    }

    sscanf(data, "GET:_itv\n0,%hhu", &count);

#ifdef DEBUG_HIGH
    printf("sizeof(*count) = %u\n", sizeof(count));
    printf("*count = %hhu\n", count);
    printf("\n");
#endif
    return 1;
}

int32_t Urg3dSensor::highBlockingGetMotorAngleTable()
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int32_t ret = highBlockingCommon(header, data, "GET:tblh\n");
    if (ret < 0) {
        return ret;
    }
    for (int i = START_POS, s = 0; i + SPOT_BYTE <= header.length - header.headerLength; i += SKIP_BYTE, ++s) {
        m_tables.spotMotorAngleRatio[s] = hexCharsToU32(data, i, SPOT_BYTE) / 65535.0;
    }

    return 1;
}

int32_t Urg3dSensor::highBlockingGetMotorAngleTableMulti(uint16_t spotCount)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    char command[URG3D_MAX_TX_LENGTH] = { 0 };

    uint8_t groupCount = spotCount / URG3D_VSSP_SPOTS_IN_GROUP + 1;
    for (int group = 0; group < groupCount; group++) {
        sprintf(command, "GET:tblh[%02d]\n", group);
        int ret = highBlockingCommon(header, data, command);
        if (ret < 0) {
            return ret;
        }
        uint16_t startPos = group * URG3D_VSSP_SPOTS_IN_GROUP;
        for (int i = MULTI_START_POS, s = startPos; i + SPOT_BYTE <= header.length - header.headerLength; i += SKIP_BYTE, ++s) {
            m_tables.spotMotorAngleRatio[s] = hexCharsToU32(data, i, SPOT_BYTE) / 65535.0;
        }
    }

    return 1;
}

int32_t Urg3dSensor::highBlockingGetRemAngleTable(uint8_t intlCount)
{
    if (m_tables.spotRemAngleLoaded[intlCount - 1] == 0) {
        URG3D_VSSP_HEADER_T header;
        char data[URG3D_MAX_RX_LENGTH] = { 0 };
        char command[URG3D_MAX_TX_LENGTH] = { 0 };
        for (int remIntlIndex = 0; remIntlIndex < intlCount; ++remIntlIndex)
        {
            sprintf(command, "GET:tv%02d\n", remIntlIndex);
            int32_t ret = highBlockingCommon(header, data, command);
            if (ret < 0) {
                return ret;
            }
            for (int i = START_POS, s = 0; i + SPOT_BYTE <= header.length - header.headerLength; i += SKIP_BYTE, ++s) {
                double intl = hexCharsToU32(data, i, SPOT_BYTE) * (2.0 * M_PI) / 65535.0;
                m_tables.spotRemAngleRad[intlCount - 1][remIntlIndex][s] = intl;
                m_tables.cosRemAngleRad[intlCount - 1][remIntlIndex][s] = cos(intl);
                m_tables.sinRemAngleRad[intlCount - 1][remIntlIndex][s] = sin(intl);
            }
        }
        m_tables.spotRemAngleLoaded[intlCount - 1] = 1;
    }

    return 1;
}

int32_t Urg3dSensor::highBlockingGetRemAngleTableMulti(uint8_t intlCount, uint16_t spotCount)
{
    if (m_tables.spotRemAngleLoaded[intlCount - 1] == 0) {
        URG3D_VSSP_HEADER_T header;
        char data[URG3D_MAX_RX_LENGTH] = { 0 };
        char command[URG3D_MAX_TX_LENGTH] = { 0 };
        uint8_t groupCount = spotCount / URG3D_VSSP_SPOTS_IN_GROUP + 1;

        for (int remIntlIndex = 0; remIntlIndex < intlCount; ++remIntlIndex)
        {
            for (int group = 0; group < groupCount; group++) {
                sprintf(command, "GET:tv%02d[%02d]\n", remIntlIndex, group);
                int32_t ret = highBlockingCommon(header, data, command);
                if (ret < 0) {
                    return ret;
                }
                uint16_t startPos = group * URG3D_VSSP_SPOTS_IN_GROUP;
                for (int i = MULTI_START_POS, s = startPos; i + SPOT_BYTE <= header.length - header.headerLength; i += SKIP_BYTE, ++s) {
                    double intl = hexCharsToU32(data, i, SPOT_BYTE) * (2.0 * M_PI) / 65535.0;
                    m_tables.spotRemAngleRad[intlCount - 1][remIntlIndex][s] = intl;
                    m_tables.cosRemAngleRad[intlCount - 1][remIntlIndex][s] = cos(intl);
                    m_tables.sinRemAngleRad[intlCount - 1][remIntlIndex][s] = sin(intl);
                }
            }
        }
        m_tables.spotRemAngleLoaded[intlCount - 1] = 1;
    }

    return 1;
}

int32_t Urg3dSensor::highBlockingSetMotorInterlaceCount(uint8_t count)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int32_t ret = 0;
    char command[URG3D_MAX_TX_LENGTH] = { 0 };

    //check
    if (count < 1 || count > m_info.spec.motorInterlaceCount) {
        return -1;
    }

    sprintf(command, "SET:_itl=0,%02d\n", count);
    ret = highBlockingCommon(header, data, command);
    if (ret < 0) {
        return ret;
    }

    return 1;
}

int32_t Urg3dSensor::highBlockingSetRemInterlaceCount(uint8_t count)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int32_t ret = 0;
    char command[URG3D_MAX_TX_LENGTH] = { 0 };

    //check
    if (count < 1 || count > m_info.spec.remInterlaceCount) {
        return -1;
    }

    sprintf(command, "SET:_itv=0,%02d\n", count);
    ret = highBlockingCommon(header, data, command);
    if (ret < 0) {
        return ret;
    }

    if (m_info.spec.spotCount > URG3D_VSSP_SPOTS_IN_GROUP) {
        highBlockingGetRemAngleTableMulti(count, m_info.spec.spotCount);
    } else {
        highBlockingGetRemAngleTable(count);
    }

    return 1;
}

int32_t Urg3dSensor::highStartData(URG3D_MEASUREMENT_TYPE meas)
{
    auto command = datCommand(meas, START);
    if (meas == URG3D_NO_REQUEST) {
        return 0;
    }
    return lowRequestCommand(command.c_str());
}

int32_t Urg3dSensor::highBlockingStartData(URG3D_MEASUREMENT_TYPE meas)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    auto command = datCommand(meas, START);
    int32_t ret = 0;

    if (meas == URG3D_NO_REQUEST) {
        return 0;
    }

    ret = highBlockingCommon(header, data, command);
    if (ret < 0) {
        return ret;
    }

    return 1;
}

int32_t Urg3dSensor::highStopData(URG3D_MEASUREMENT_TYPE meas)
{
    auto command = datCommand(meas, STOP);
    if (meas == URG3D_NO_REQUEST) {
        return 0;
    }
    return lowRequestCommand(command.c_str());
}

int32_t Urg3dSensor::highBlockingStopData(URG3D_MEASUREMENT_TYPE meas)
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    auto command = datCommand(meas, STOP);
    int32_t ret = 0;

    if (meas == URG3D_NO_REQUEST) {
        return 0;
    }

    ret = highBlockingCommon(header, data, command);
    if (ret < 0) {
        return ret;
    }

    return 1;
}

int32_t Urg3dSensor::highGetMeasurementData(URG3D_MEASUREMENT_DATA_T& data)
{
    if ((m_isInit == URG3D_FALSE) || (data.spots.size() != m_info.spec.spotCount)) {
        return URG3D_NOT_INITIALIZED;
    }

    int32_t ret = 0;
    string type;

    ret = lowGetMeasurementData(m_vsspHeader, m_rangeHeader, m_rangeIndex,
                                m_rawRange, m_rawRangeIntensity, type);
    if (ret <= 0) {
        return 0;
    }

    setCommonMeasurementData(data, m_vsspHeader, m_rangeHeader, m_rangeIndex,
                             m_rawRange, m_rawRangeIntensity, type);

    return 1;
}

int32_t Urg3dSensor::highGetAuxiliaryData(URG3D_AUXILIARY_DATA_T& data)
{
    int ret = 0;
    int i = 0, point = 0;
    URG3D_VSSP_HEADER_T libHeader;
    URG3D_AX_HEADER_T libAxHeader;
    URG3D_DATA_AX_T libAxData;

    ret = lowGetAx(libHeader, libAxHeader, libAxData);
    if (ret <= 0) {
        return ret;
    }

    data.timestampMs = libAxHeader.timestampMs;
    data.type = URG3D_NO_RECORD;
    data.type = static_cast<URG3D_AUXILIARY_TYPE>(data.type | (((libAxHeader.dataBitfield & 0xe0000000) == 0xe0000000) ? URG3D_GYRO_DATA : URG3D_NO_RECORD));
    data.type = static_cast<URG3D_AUXILIARY_TYPE>(data.type | (((libAxHeader.dataBitfield & 0x1c000000) == 0x1c000000) ? URG3D_ACCEL_DATA : URG3D_NO_RECORD));
    data.type = static_cast<URG3D_AUXILIARY_TYPE>(data.type | (((libAxHeader.dataBitfield & 0x03800000) == 0x03800000) ? URG3D_COMPASS_DATA : URG3D_NO_RECORD));
    data.type = static_cast<URG3D_AUXILIARY_TYPE>(data.type | (((libAxHeader.dataBitfield & 0x00400000) == 0x00400000) ? URG3D_TEMPERATURE_DATA : URG3D_NO_RECORD));
    data.recordCount = libAxHeader.dataCount;
    strncpy(data.status, libHeader.status, sizeof(data.status));

    //clear
    memset(&data.records, 0, sizeof(URG3D_AUXILIARY_RECORD_T) * URG3D_MAX_AUX_COUNT);

    //set
    for (i = 0; i < data.recordCount; i++) {
        data.records[i].timestampMs = data.timestampMs + libAxHeader.dataMs * i;

        if (libAxHeader.dataBitfield & (1 << 31)) {
            data.records[i].gyroX = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 30)) {
            data.records[i].gyroY = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 29)) {
            data.records[i].gyroZ = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 28)) {
            data.records[i].accelX = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 27)) {
            data.records[i].accelY = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 26)) {
            data.records[i].accelZ = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 25)) {
            data.records[i].compassX = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 24)) {
            data.records[i].compassY = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 23)) {
            data.records[i].compassZ = libAxData.value[point];
            ++point;
        }
        if (libAxHeader.dataBitfield & (1 << 22)) {
            data.records[i].temperature = libAxData.value[point];
            ++point;
        }
    }

#ifdef DEBUG_HIGH
    printf("data.timestampMs = %lu\n", data.timestampMs);
    printf("data.status = %.3s\n", data.status);
    printf("data.type = %d\n", data.type);
    printf("data.recodeCount = %u\n", data.recordCount);
    for (i = 0; i < data.recordCount; i++) {
        printf("data.records[%d].timestampMs = %lu\n", i, data.records[i].timestampMs);
        printf("data.records[%d].gyroX = %ld\n", i, data.records[i].gyroX);
        printf("data.records[%d].gyroY = %ld\n", i, data.records[i].gyroY);
        printf("data.records[%d].gyroZ = %ld\n", i, data.records[i].gyroZ);
        printf("data.records[%d].accelX = %ld\n", i, data.records[i].accelX);
        printf("data.records[%d].accelY = %ld\n", i, data.records[i].accelY);
        printf("data.records[%d].accelX = %ld\n", i, data.records[i].accelZ);
        printf("data.records[%d].compassX = %ld\n", i, data.records[i].compassX);
        printf("data.records[%d].compassY = %ld\n", i, data.records[i].compassY);
        printf("data.records[%d].compassZ = %ld\n", i, data.records[i].compassZ);
        printf("data.records[%d].temperature = %ld\n", i, data.records[i].temperature);
    }
    printf("\n");
#endif

    return 1;
}

int32_t Urg3dSensor::highBlockingRestart()
{
    URG3D_VSSP_HEADER_T header;
    char data[URG3D_MAX_RX_LENGTH] = { 0 };
    int ret = 0;

    ret = highBlockingCommon(header, data, "RST\n");
    if (ret < 0) {
        return ret;
    }

    return 1;
}

int32_t Urg3dSensor::highGetSensorInfo(URG3D_SENSOR_INFO_T& info)
{
    if (!m_isInit) {
        return URG3D_NOT_INITIALIZED;
    }

    memcpy(&info, &m_info, sizeof(URG3D_SENSOR_INFO_T));

    return 1;
}

int32_t Urg3dSensor::highBlockingCommon(URG3D_VSSP_HEADER_T& header,
                                        char* const data,
                                        const string &command,
                                        const string &dataHead)
{
    int32_t lengthData = 0;
    int64_t startTimeMs;

    if (command.length() < URG3D_TYPE_LENGTH) {
        return -1;
    }

    if (lowRequestCommand(command.c_str()) < 0) {
        return -1;
    }

    char type[URG3D_TYPE_LENGTH];
    memcpy(type, command.c_str(), URG3D_TYPE_LENGTH);

    startTimeMs = Urg3dTicks::Urg3dTicksMs();
    while (1) {
        while (lowGetBinary(header, data, lengthData) > 0) {
            if (strncmp(header.type, "ERR", URG3D_TYPE_LENGTH) == 0) {
                return -1;
            }
            if (strncmp(header.type, "_er", URG3D_TYPE_LENGTH) == 0) {
                return -1;
            }
            if (strncmp(header.type, type, URG3D_TYPE_LENGTH) == 0) {
                if (dataHead.empty()) {
                    if (strncmp(data, command.c_str(), command.length() - 1) == 0) {
                        return 1;
                    }
                } else {
                    if (strncmp(data, dataHead.c_str(), dataHead.length() - 1) == 0) {
                        return 1;
                    }
                }
            }
        }
        if (Urg3dTicks::Urg3dTicksMs() - startTimeMs >= m_blockingTimeoutMs) {
            return -2;
        }
#ifdef URG3D_WINDOWS_OS
        Sleep(10);
#else
        usleep(10000);
#endif
    }
}

void Urg3dSensor::lowPurge()
{
    m_connection->clearRing();
}

int32_t Urg3dSensor::lowGetBinary(URG3D_VSSP_HEADER_T& header,
                                  char* const data,
                                  int& lengthData)
{
    int ret = 0;
    string type, status;

    if ((ret = nextReceiveReady(type, status)) <= 0) {
        return ret;
    }

    //copy
    auto *rb = m_connection->ringBuffer();
    rb->read((char*)&header, URG3D_VSSP_HEADER_LENGTH);
    lengthData = header.length - header.headerLength;
    if (lengthData != 0) {
        rb->read((char*)data, lengthData);
    }
    data[lengthData] = '\0';

    nextReceiveFlagClear();

#ifdef DEBUG_LOW
    printf("sizeof(header) = %d\n", sizeof(header));
    printf("header.mark = %.4s\n", header.mark);
    printf("header.type = %.3s\n", header.type);
    printf("header.status  = %.3s\n", header.status);
    printf("header.header_length = %u\n", header.headerLength);
    printf("header.length = %u\n", header.length);
    printf("header.received_time_ms = %lu\n", header.receivedTimeMs);
    printf("header.send_time_ms = %lu\n", header.sendTimeMs);
    printf("*lengthData = %d\n", lengthData);
    printf("strlen(data) = %u\n", strlen(data));
    printf("data = %s\n", data);

    printf("\n");
#endif

    return 1;
}

void Urg3dSensor::setConstantParameter(URG3D_SENSOR_SPEC_T &spec)
{
    spec.motorInterlaceCount = URG3D_MAX_MOTOR_INTERLACE_COUNT;
    spec.remInterlaceCount = URG3D_MAX_REM_INTERLACE_COUNT;
    spec.firstLineNumber = URG3D_FIRST_LINE_NUMBER;
    spec.lastLineNumber = URG3D_LAST_LINE_NUMBER;
    spec.lineCount = URG3D_LINE_COUNT;
    spec.spotCount = URG3D_MAX_SPOTS_COUNT;
    spec.echoCount = URG3D_MAX_ECHOS_COUNT;
    spec.accelScale = URG3D_ACCEL_SCALE;
    spec.gyroScale = URG3D_GYRO_SCALE;
    spec.regionCount = URG3D_REGION_COUNT;
    spec.isLsbResult = URG3D_IS_LSB_RESULT;
    spec.swingDir = URG3D_SWING_DIR;
}

void Urg3dSensor::resizeMembers(URG3D_MEASUREMENT_DATA_T& mData)
{
    uint8_t remInterlaceCount = m_info.spec.remInterlaceCount;
    uint16_t spotCount = m_info.spec.spotCount;
    uint8_t  echoCount = m_info.spec.echoCount;
    uint8_t  regionCount = m_info.spec.regionCount;

    // resize transform table vectors
    m_tables.spotMotorAngleRatio.resize(spotCount);
    m_tables.spotRemAngleLoaded.resize(remInterlaceCount);
    m_tables.spotRemAngleRad.resize(remInterlaceCount);
    m_tables.cosRemAngleRad.resize(remInterlaceCount);
    m_tables.sinRemAngleRad.resize(remInterlaceCount);
    for (int i = 0; i < remInterlaceCount; ++i) {
        m_tables.spotRemAngleRad[i].resize(remInterlaceCount);
        m_tables.cosRemAngleRad[i].resize(remInterlaceCount);
        m_tables.sinRemAngleRad[i].resize(remInterlaceCount);
        for (int j = 0; j < remInterlaceCount; ++j) {
            m_tables.spotRemAngleRad[i][j].resize(spotCount);
            m_tables.cosRemAngleRad[i][j].resize(spotCount);
            m_tables.sinRemAngleRad[i][j].resize(spotCount);
        }
    }

    // resize raw data
    m_rawRange.raw.resize(spotCount * echoCount);
    m_rawRangeIntensity.raw.resize(spotCount * echoCount);
    m_rangeIndex.index.resize(spotCount + 1);

    // resize VSSP headers
    m_rangeHeader.totalDetectionArea.resize(regionCount);

    // resize measurement data
    mData.spots.resize(spotCount);
    mData.totalDetectionArea.resize(regionCount);
    for (int count = 0; count < spotCount; ++count) {
        mData.spots[count].polar.resize(echoCount);
        mData.spots[count].point.resize(echoCount);
        mData.spots[count].detectionResult.resize(echoCount);
        for (int echo = 0; echo < echoCount; ++echo) {
            mData.spots[count].detectionResult[echo].isDetected.resize(regionCount);
        }
    }
}

void Urg3dSensor::nextReceiveFlagClear()
{
    m_nextPacket.nextHeaderReady = 0;
    m_nextPacket.nextDataReady = 0;
}

string Urg3dSensor::datCommand(URG3D_MEASUREMENT_TYPE meas, uint8_t state)
{
    string dat[5] = { "",
        "DAT:ro=" + std::to_string(state) + "\n",
        "DAT:ri=" + std::to_string(state) + "\n",
        "DAT:ax=" + std::to_string(state) + "\n",
        "DAT:ra=" + std::to_string(state) + "\n"};
    return dat[meas];
}

uint32_t Urg3dSensor::hexCharsToU32(char* hex
                                    , int32_t begin
                                    , int32_t length)
{
    int32_t i;
    uint32_t ret = 0;
    for (i = 0; i < length; ++i) {
        if (hex[begin + i] >= '0' && hex[begin + i] <= '9') {
            ret |= ((unsigned int)hex[begin + i] - '0') << ((length - 1 - i) * 4);
        }
        if (hex[begin + i] >= 'A' && hex[begin + i] <= 'F') {
            ret |= ((unsigned int)hex[begin + i] - 'A' + 10) << ((length - 1 - i) * 4);
        }
        if (hex[begin + i] >= 'a' && hex[begin + i] <= 'f') {
            ret |= ((unsigned int)hex[begin + i] - 'a' + 10) << ((length - 1 - i) * 4);
        }
    }
    return ret;
}

int32_t Urg3dSensor::hexCharsToS32(char* hex, int32_t begin, int32_t length)
{
    return (int32_t)hexCharsToU32(hex, begin, length);
}

int32_t Urg3dSensor::setCommonMeasurementData(URG3D_MEASUREMENT_DATA_T& data,
                                              const URG3D_VSSP_HEADER_T& libHeader,
                                              URG3D_RANGE_HEADER_T& libRangeHeader,
                                              URG3D_RANGE_INDEX_T& libRangeIndex,
                                              const URG3D_DATA_RANGE_T& libDataRange,
                                              const URG3D_DATA_RANGE_INTENSITY_T& libDataRangeIntensity,
                                              const string& type)
{
    int i = 0;
    int spot, echo;
    bool ri = (type == "_ri");
    bool ra = (type == "_ra");

    data.timestamp = libRangeHeader.lineHeadTimestamp;
    data.frameNumber = libRangeHeader.frame;
    data.motorFieldNumber = libRangeHeader.motorField;
    data.remFieldNumber = libRangeHeader.remField;
    data.lineNumber = libRangeHeader.line;
    data.spotCount = libRangeIndex.nspots + libRangeHeader.spot;
    strncpy(data.status, libHeader.status, sizeof(data.status));
    data.output = ra ? libRangeHeader.output : 0;
    data.input = ra ? libRangeHeader.input : 0;
    data.dirtState = ra ? libRangeHeader.dirtInfo >> 14 : 0;
    data.dirtRatioX10 = ra ? libRangeHeader.dirtInfo & 0x3fff : 0;
    data.dustState = ra ? libRangeHeader.dustInfo >> 14 : 0;
    data.dustRatioX10 = ra ? libRangeHeader.dustInfo & 0x3fff : 0;
    for (int i = 0; i < m_info.spec.regionCount; i++) {
        data.totalDetectionArea[i] = ra ? libRangeHeader.totalDetectionArea[i] : 0;
    }

    // Prevention of out-of-range access
    if (libRangeHeader.remField > m_info.spec.remInterlaceCount
            || libRangeHeader.remField < 0) {
        data.remFieldNumber = 0;
    }
    if (libRangeHeader.remInterlace > m_info.spec.remInterlaceCount
            || libRangeHeader.remInterlace <= 0) {
        libRangeHeader.remInterlace = 1;
    }

    uint16_t start = libRangeHeader.spot;
    uint16_t end = start + libRangeIndex.nspots;

    for (spot = start; spot < end; ++spot) {
        const double motorRad = ((double)libRangeHeader.lineHeadMotorAngleRatio
                                 + (double)(libRangeHeader.lineTailMotorAngleRatio - libRangeHeader.lineHeadMotorAngleRatio)
                                 * m_tables.spotMotorAngleRatio[spot]) / 65535 * 2 * M_PI;
        const double cosMotorRad = cos(motorRad);
        const double sinMotorRad = sin(motorRad);

        double tempRemRad = m_tables.spotRemAngleRad[libRangeHeader.remInterlace - 1][data.remFieldNumber][spot];
        if (tempRemRad > M_PI) {
            tempRemRad -= 2.0 * M_PI;
        }
        const double remRad = tempRemRad;
        const double cosRemRad = m_tables.cosRemAngleRad[libRangeHeader.remInterlace - 1][data.remFieldNumber][spot];
        const double sinRemRad = m_tables.sinRemAngleRad[libRangeHeader.remInterlace - 1][data.remFieldNumber][spot];

        //// By swing_dir equals 0, the direction of rotation of the motor is horizontal.
        double xRad = cosRemRad * cosMotorRad;
        double yRad = cosRemRad * sinMotorRad;
        double zRad = sinRemRad;
        if (m_info.spec.swingDir) {
            xRad = cosRemRad * cosMotorRad;
            yRad = sinRemRad;
            zRad = cosRemRad * sinMotorRad;
        }

        // Prevention of out-of-range access
        int echoCount = libRangeIndex.index[spot + 1] - libRangeIndex.index[spot];
        if (echoCount < 0 || echoCount > m_info.spec.echoCount) {
            echoCount = 0;
            libRangeIndex.index[spot + 1] = 0;
            libRangeIndex.index[spot] = 0;
        }
        data.spots[spot].echoCount = echoCount;
        for (echo = 0; echo < data.spots[spot].echoCount; ++echo) {
            data.spots[spot].polar[echo].rangeM = 0;
            data.spots[spot].polar[echo].verticalRad = m_info.spec.swingDir ? motorRad : remRad;
            data.spots[spot].polar[echo].horizontalRad = m_info.spec.swingDir ? remRad : motorRad;
            data.spots[spot].polar[echo].intensity = 0;
        }
        for (i = libRangeIndex.index[spot], echo = 0; i < libRangeIndex.index[spot + 1]; ++i, ++echo) {
            if (m_info.spec.isLsbResult > 0) {
                for (int region = 0; region < m_info.spec.regionCount; ++region) {
                    data.spots[spot].detectionResult[echo].isDetected[region] = (libDataRangeIntensity.raw[i].intensity & (1 << region)) > 0 ? 1 : 0;
                }
            }
            const double rangeM = ((ri || ra) ? libDataRangeIntensity.raw[i].rangeMm : libDataRange.raw[i].rangeMm) / 1000.0;
            data.spots[spot].polar[echo].rangeM = rangeM;
            data.spots[spot].polar[echo].intensity = (ri || ra) ? libDataRangeIntensity.raw[i].intensity : 0;

            data.spots[spot].point[echo].xm = rangeM * xRad;
            data.spots[spot].point[echo].ym = rangeM * yRad;
            data.spots[spot].point[echo].zm = rangeM * zRad;
            data.spots[spot].point[echo].intensity = data.spots[spot].polar[echo].intensity;
        }
    }

#ifdef DEBUG_HIGH
    for (int spot = 0; spot < data.spotCount; ++spot) {
        for (int echo = 0; echo < data.spots[spot].echoCount; ++echo) {
            printf("data.spots[%d].point[%d].xm = %lf\n", spot, echo, data.spots[spot].point[echo].xm);
            printf("data.spots[%d].point[%d].ym = %lf\n", spot, echo, data.spots[spot].point[echo].ym);
            printf("data.spots[%d].point[%d].zm = %lf\n", spot, echo, data.spots[spot].point[echo].zm);
            printf("data.spots[%d].polar[%d].rangeM = %lf\n", spot, echo, data.spots[spot].polar[echo].rangeM);
            printf("data.spots[%d].polar[%d].verticalRad = %lf\n", spot, echo, data.spots[spot].polar[echo].verticalRad);
            printf("data.spots[%d].polar[%d].horizontalRad = %lf\n", spot, echo, data.spots[spot].polar[echo].horizontalRad);
            printf("data.spots[%d].polar[%d].intensity = %u\n", spot, echo, data.spots[spot].polar[echo].intensity);
        }
    }
    printf("\n");
#endif

    data.isLineDataReady = (end == m_info.spec.spotCount) ? 1 : 0;

    return 1;
}

int32_t Urg3dSensor::lowRequestCommand(const char* const command)
{
    return m_connection->sendData(command, strlen(command));
}

int32_t Urg3dSensor::lowGetMeasurementData(URG3D_VSSP_HEADER_T &header, URG3D_RANGE_HEADER_T &rangeHeader,
                                           URG3D_RANGE_INDEX_T &rangeIndex, URG3D_DATA_RANGE_T& dataRange,
                                           URG3D_DATA_RANGE_INTENSITY_T &dataRangeIntensity, string& type)
{
    int ret = 0;
    string status;

    if ((ret = nextReceiveReady(type, status)) <= 0) {
        return ret;
    }

    //check
    if ((type == "_ro") || (type == "_ri") || (type == "_ra")) {
        lowGetRangeHeader(header, rangeHeader, rangeIndex);
        auto *rb = m_connection->ringBuffer();

        if (type == "_ro") {
            //copy data_range
            rb->read(reinterpret_cast<char*>(dataRange.raw.data()),
                     rangeIndex.index[rangeIndex.nspots] * sizeof(URG3D_RAW_RANGE_T));
        } else {
            //copy data_range_intensity
            rb->read(reinterpret_cast<char*>(dataRangeIntensity.raw.data()),
                     rangeIndex.index[rangeIndex.nspots] * sizeof(URG3D_RAW_RANGE_INTENSITY_T));
        }
        nextReceiveFlagClear();

        return 1;
    }

    return 0;
}

int32_t Urg3dSensor::lowGetAx(URG3D_VSSP_HEADER_T& header,
                              URG3D_AX_HEADER_T& axHeader,
                              URG3D_DATA_AX_T& axData)
{
    int ret = 0;
    int count = 0;
    char buf[URG3D_MAX_RX_LENGTH] = { 0 };
    string type, status;

    if ((ret = nextReceiveReady(type, status)) <= 0) {
        return ret;
    }

    //check
    if (type != string("_ax")) {
        return 0;
    }

    //copy vssp_header
    auto *rb = m_connection->ringBuffer();
    rb->read((char*)&header, URG3D_VSSP_HEADER_LENGTH);

    //copy ax_header
    rb->read(buf, 12);
    axHeader.headerLength = (*(uint16_t*)(buf + 0));
    axHeader.timestampMs = (*(uint32_t*)(buf + 2));
    axHeader.dataBitfield = (*(uint32_t*)(buf + 6));
    axHeader.dataCount = (*(uint8_t*)(buf + 10));
    axHeader.dataMs = (*(uint8_t*)(buf + 11));

    //copy ax_data
    count = (header.length - header.headerLength - axHeader.headerLength) / sizeof(int32_t);
    rb->read(reinterpret_cast<char*>(&axData.value), count * sizeof(int32_t));

    nextReceiveFlagClear();

#ifdef DEBUG_LOW
    printf("sizeof(header) = %d\n", sizeof(header));
    printf("header.mark = %.4s\n", header.mark);
    printf("header.type = %.3s\n", header.type);
    printf("header.status = %.3s\n", header.status);
    printf("header.headerLength = %u\n", header.headerLength);
    printf("header.length = %u\n", header.length);
    printf("header.receivedTimeMs = %lu\n", header.receivedTimeMs);
    printf("header.sendTimeMs = %lu\n", header.sendTimeMs);
    printf("sizeof(axHeader) = %d\n", sizeof(axHeader));
    printf("axHeader.headerLength = %u\n", axHeader.headerLength);
    printf("axHeader.timestampMs = %lu\n", axHeader.timestampMs);
    printf("axHeader.dataBitfield = 0x%lx\n", axHeader.dataBitfield);
    printf("axHeader.dataCount = %d\n", axHeader.dataCount);
    printf("axHeader.dataMs = %d\n", axHeader.dataMs);
    printf("sizeof(axData) = %d\n", sizeof(axData));
    for (int i = 0; i < count; i++) {
        printf("axData.value[%d] = %ld\n", i, axData.value[i]);
    }
    printf("\n");
#endif

    return 1;
}

int32_t Urg3dSensor::lowGetRangeHeader(URG3D_VSSP_HEADER_T& header,
                                       URG3D_RANGE_HEADER_T& rangeHeader,
                                       URG3D_RANGE_INDEX_T& rangeIndex)
{
    char buf[URG3D_MAX_RX_LENGTH] = { 0 };

    auto *rb = m_connection->ringBuffer();
    //copy
    rb->read((char*)&header, URG3D_VSSP_HEADER_LENGTH);

    //copy range_header
    rb->read(buf, 2);
    rangeHeader.headerLength = (*(uint16_t*)(buf + 0));
    rb->read(buf, rangeHeader.headerLength - 2);
    rangeHeader.lineHeadTimestamp = (*(uint32_t*)(buf + 0));
    rangeHeader.lineTailTimestamp = (*(uint32_t*)(buf + 4));
    rangeHeader.lineHeadMotorAngleRatio = (*(int16_t*)(buf + 8));
    rangeHeader.lineTailMotorAngleRatio = (*(int16_t*)(buf + 10));
    rangeHeader.frame = (*(uint8_t*)(buf + 12));
    rangeHeader.motorField = (*(uint8_t*)(buf + 13));
    rangeHeader.line = (*(uint16_t*)(buf + 14));
    rangeHeader.spot = (*(uint16_t*)(buf + 16));
    if (rangeHeader.headerLength == 20) {
        rangeHeader.remField = 0;
        rangeHeader.remInterlace = 1;
    } else {
        rangeHeader.remField = (*(uint8_t*)(buf + 18));
        rangeHeader.remInterlace = (*(uint8_t*)(buf + 19));
    }

    // If the header size exceeds 24, the response of the ra command.
    if (rangeHeader.headerLength > 24) {
        rangeHeader.dustInfo = (*(uint16_t*)(buf + 20));
        rangeHeader.output = (*(uint8_t*)(buf + 22));
        rangeHeader.input = (*(uint8_t*)(buf + 23));
        rangeHeader.dirtInfo = (*(uint16_t*)(buf + 24));
        for (int i = 0; i < m_info.spec.regionCount; i++) {
            rangeHeader.totalDetectionArea[i] = (*(uint32_t*)(buf + 26 + i * sizeof(uint32_t)));
        }
    }

    //copy range_index
    rb->read(buf, 4);
    rangeIndex.indexLength = (*(uint16_t*)(buf + 0));
    rangeIndex.nspots = (*(uint16_t*)(buf + 2));
    rb->read(reinterpret_cast<char*>(rangeIndex.index.data()),
             (rangeIndex.nspots + 1) * sizeof(uint16_t));

    //skip reserve
    if (rangeIndex.nspots % 2 == 0) {
        rb->read(buf, 2);
    }

    // Prevention of out-of-range access
    if ((rangeIndex.nspots > m_info.spec.spotCount)
            || (rangeIndex.nspots < 0)) {
        rangeIndex.nspots = 0;
    }

    return 1;
}
