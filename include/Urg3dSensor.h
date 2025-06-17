#ifndef URG3DSENSOR_H
#define URG3DSENSOR_H

#include "Urg3d_t.h"
#include "Urg3dDetectOS.h"
#include <string>
#include <memory>
#include <sstream>
#include <iostream>

class Urg3dConnection;

class Urg3dSensor
{
public:
    enum URG3D_CONNECTION_TYPE {
        TCP = 0,
    };

    enum URG3D_STATE {
        URG3D_FALSE = 0,
        URG3D_TRUE = 1,
    };

    enum URG3D_MEASUREMENT_STATE {
        STOP = 0,
        START = 1,
    };

    enum URG3D_ANGLE_TABLE {
        SPOT_BYTE = 4,
        SKIP_BYTE = 5,
        START_POS = 9,
        MULTI_START_POS = 13,
    };

    Urg3dSensor(URG3D_CONNECTION_TYPE type);
    ~Urg3dSensor() = default;

    Urg3dSensor(const Urg3dSensor &other) = delete;
    Urg3dSensor(Urg3dSensor &&other) noexcept = default;

    Urg3dSensor& operator=(const Urg3dSensor &rhs) = delete;
    Urg3dSensor& operator=(Urg3dSensor &&rhs) noexcept = default;

    /*!
      ==================== 3D-URG Common ====================
    */


    /*!
      \brief open the connection to the sensor

      \param[in] ip : IP address expressed in string, i.e. ip = "192.168.0.10"
      \param[in] port :  port number expressed in integer, i.e. port = 10940

      \retval 0 succeeded
      \retval other error
    */
    int32_t open(const char* ip, int port);

    /*!
      \brief close the connection to the sensor

      \retval 0 succeeded
      \retval other error
    */
    int32_t close();

    /*!
     * \brief Indicates the state of connection with the sensor
     * \retval 0 : disconnected
     *         1 : connected
     */
    int8_t isActive();

    /*!
      \brief check the next data has received

      \param[out] type   : received packet type
      \param[out] status : VSSP status

      \retval 0 unreceived
      \retval other received
    */
    int32_t nextReceiveReady(string& type, string& status);

    /*!
      ==================== 3D-URG High Layer ====================
    */

    /*!
      \brief set blocking function timeout

      \param[in] timeoutMs : timeout ms, i.e. t_ms = 1000
    */
    void highSetBlockingTimeoutMs(uint32_t timeoutMs);

    /*!
      \brief initialize the urg3d session

      \param[out] data : URG3D_MEASUREMENT_DATA_T struct

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingInit(URG3D_MEASUREMENT_DATA_T& mData);

    /*!
      \brief wait to finish initialize of mesurement

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingWaitFinishedInitialize();

    /*!
      \brief get sensor-specific values (VSSP >= 2.3)

      \param[out] spec : URG3D_SENSOR_SPEC_T struct

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingGetSensorSpecification(URG3D_SENSOR_SPEC_T& spec);

    /*!
      \brief receive version information from device

      \param[out] version : URG3D_SENSOR_VERSION_T struct

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingGetSensorVersion(URG3D_SENSOR_VERSION_T& version);

    /*!
      \brief receive motor interlace count from device

      \param[out] count : interlace count

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingGetMotorInterlaceCount(uint8_t& count);

    /*!
      \brief receive rem interlace count from device

      \param[out] count : interlace count

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingGetRemInterlaceCount(uint8_t& count);

    /*!
      \brief receive motor angle table from device.
             use this function for sensors with 256 or fewer spots per line,
             or for sensors with VSSP version 2.2 or earlier.
             see urg3dHighBlockingInit().

      \retval 1 succeeded
      \retval -1 received error message
    */
    int32_t highBlockingGetMotorAngleTable();

    /*!
      \brief receive motor angle table from device.
             use this function for sensors with more than 256 spots per line
             and with VSSP version 2.3 or later.
             see urg3dHighBlockingInit().

      \param[in] spotCount : spot count

      \retval 1 succeeded
      \retval -1 received error message
    */
    int32_t highBlockingGetMotorAngleTableMulti(uint16_t spotCount);

    /*!
      \brief receive rem angle table from device.
             use this function for sensors with 256 or fewer spots per line,
             or for sensors with VSSP version 2.2 or earlier.
             see urg3dHighBlockingInit().

      \param[in] intlCount : rem interlace count

      \retval 1 succeeded
      \retval -1 received error message
    */
    int32_t highBlockingGetRemAngleTable(uint8_t intlCount);

    /*!
      \brief receive rem angle table from device.
             use this function for sensors with more than 256 spots per line
             and with VSSP version 2.3 or later.
             see urg3dHighBlockingInit().

      \param[in] intlCount : rem interlace count
      \param[in] spotCount : spot count

      \retval 1 succeeded
      \retval -1 received error message
    */
    int32_t highBlockingGetRemAngleTableMulti(uint8_t intlCount, uint16_t spotCount);

    /*!
      \brief send motor interlace count for device

      \param[in] count : interlace count, i.e. count = 4

      \retval 1 succeeded
      \retval -1 received error message or settting count is out of range
      \retval -2 connection timeout
    */
    int32_t highBlockingSetMotorInterlaceCount(uint8_t count);

    /*!
      \brief send rem interlace count for device

      \param[in] count : interlace count, i.e. count = 4

      \retval 1 succeeded
      \retval -1 received error message or settting count is out of range
      \retval -2 connection timeout
    */
    int32_t highBlockingSetRemInterlaceCount(uint8_t count);

    /*!
      \brief request to start data (do not wait for a response to the start command)

      \param[in] meas : URG3D_MEASUREMENT_TYPE sturct, i.e. meas = URG3D_DISTANCE_INTENSITY

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t highStartData(URG3D_MEASUREMENT_TYPE meas);

    /*!
      \brief request to start data

      \param[in] meas : URG3D_MEASUREMENT_TYPE sturct, i.e. meas = URG3D_DISTANCE_INTENSITY

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t highBlockingStartData(URG3D_MEASUREMENT_TYPE meas);

    /*!
      \brief request to stop data (do not wait for a response to the stop command)

      \param[in] meas : URG3D_MEASUREMENT_TYPE sturct, i.e. meas = URG3D_DISTANCE_INTENSITY

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t highStopData(URG3D_MEASUREMENT_TYPE meas);

    /*!
      \brief request to stop data

      \param[in] meas : URG3D_MEASUREMENT_TYPE sturct, i.e. meas = URG3D_DISTANCE_INTENSITY

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t highBlockingStopData(URG3D_MEASUREMENT_TYPE meas);

    /*!
      \brief get "range and intensity" or "range only" user format from revceived data buffer

      \param[out] data : URG3D_MEASUREMENT_DATA_T struct

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t highGetMeasurementData(URG3D_MEASUREMENT_DATA_T& data);

    /*!
      \brief get "auxiliary" user format from revceived data buffer

      \param[out] data : URG3D_AUXILIARY_DATA_T struct

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t highGetAuxiliaryData(URG3D_AUXILIARY_DATA_T& data);

    /*!
      \brief restart the device

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 connection timeout
    */
    int32_t highBlockingRestart();

    /*!
      \brief get sensor information.
             Initialization must be done before using this function.
             see urg3dHighBlockingInit().

      \param[out] info : URG3D_SENSOR_INFO_T struct

      \retval 1 succeeded
      \retval -3 not initialized
    */
    int32_t highGetSensorInfo(URG3D_SENSOR_INFO_T& info);

    /*!
      \brief read the VSSP packet and split it into the VSSP common header and body.

      \param[out] header   : URG3D_VSSP_HEADER_T struct
      \param[out] data     : packet body
      \param[out] command  : VSSP command
      \param[out] dataHead : starting string of the packet you want to read

      \retval 1 succeeded
      \retval -1 received error message
      \retval -2 timeout
    */
    int32_t highBlockingCommon(URG3D_VSSP_HEADER_T& header,
                               char* const data,
                               const string &command,
                               const string &dataHead = "");

    /*!
      ==================== 3D-URG Low Layer ====================
    */

    /*!
      \brief purge the data buffer
    */
    void lowPurge();

    /*!
      \brief get binary data from revceived data buffer

      \param[out] header     : URG3D_VSSP_HEADER_T struct
      \param[out] data       : binary data
      \param[out] lengthData : the number of byte for data

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t lowGetBinary(URG3D_VSSP_HEADER_T& header,
                         char* const data,
                         int& lengthData);

    /*!
      \brief send request command to device

      \param[in] command  : VSSP command, i.e. command[] = "VER"

      \retval >=0 the number of data wrote
      \retval <0 error
    */
    int32_t lowRequestCommand(const char* const command);

private:
    /*!
      \brief set fixed values for the sensor specification parameters.
             use for sensors with VSSP version 2.2 or earlier.
             see urg3dHighBlockingInit().

      \param[out] spec : URG3D_SENSOR_SPEC_T struct
    */
    void setConstantParameter(URG3D_SENSOR_SPEC_T& spec);

    /*!
      \brief resize the variable.
             see urg3dHighBlockingInit().

      \param[out] mData : URG3D_MEASUREMENT_DATA_T struct
    */
    void resizeMembers(URG3D_MEASUREMENT_DATA_T& mData);

    
    /*!
      \brief clear the flags enabled by nextReceiveReady.
             see nextReceiveReady().
    */
    void nextReceiveFlagClear();

    /*!
      \brief clear the flags enabled by nextReceiveReady.
      
      \param[in] meas  : type of measurement.
      \param[in] state : state of measurement. 1 means start, 0 means stop.
    */
    string datCommand(URG3D_MEASUREMENT_TYPE meas, uint8_t state);

    uint32_t hexCharsToU32(char* hex, int32_t begin, int32_t length);
    int32_t hexCharsToS32(char* hex, int32_t begin, int32_t length);

    /*!
    * \brief set VSSP data to URG3D_MEASUREMENT_DATA_T

    * \param[out] data                  : URG3D_MEASUREMENT_DATA_T struct
    * \param[in]  libHeader             : URG3D_VSSP_HEADER_T struct
    * \param[out] libRangeHeader        : URG3D_RANGE_HEADER_T struct
    * \param[out] libRangeIndex         : URG3D_RANGE_INDEX_T struct
    * \param[in]  libDataRange          : URG3D_DATA_RANGE_T struct
    * \param[in]  libDataRangeIntensity : URG3D_DATA_RANGE_INTENSITY_T struct
    * \param[in]  type                  : packet type
    * 
    * \retval 1 succeeded
    */
    int32_t setCommonMeasurementData(URG3D_MEASUREMENT_DATA_T& data,
                                     const URG3D_VSSP_HEADER_T& libHeader,
                                     URG3D_RANGE_HEADER_T& libRangeHeader,
                                     URG3D_RANGE_INDEX_T& libRangeIndex,
                                     const URG3D_DATA_RANGE_T& libDataRange,
                                     const URG3D_DATA_RANGE_INTENSITY_T& libDataRangeIntensity,
                                     const string& type);

    /*!
      ==================== 3D-URG Low Layer ====================
    */

    /*!
      \brief get raw format measurement data from revceived data buffer

      \param[out] header      : URG3D_VSSP_HEADER_T struct
      \param[out] rangeHeader : URG3D_RANGE_HEADER_T struct
      \param[out] rangeIndex  : URG3D_RANGE_INDEX_T struct
      \param[out] dataRange   : URG3D_DATA_RANGE_T struct
      \param[out] dataRangeIntensity : URG3D_DATA_RANGE_INTENSITY_T struct
      \param[out] type : packet type

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t lowGetMeasurementData(URG3D_VSSP_HEADER_T& header,
                                  URG3D_RANGE_HEADER_T& rangeHeader,
                                  URG3D_RANGE_INDEX_T& rangeIndex,
                                  URG3D_DATA_RANGE_T& dataRange,
                                  URG3D_DATA_RANGE_INTENSITY_T& dataRangeIntensity,
                                  string& type);

    /*!
      \brief get auxiliary raw format from received data buffer

      \param[out] header  : URG3D_VSSP_HEADER_T struct
      \param[out] axHeader: URG3D_AX_HEADER_T struct
      \param[out] axData  : URG3D_DATA_AX_T sruct

      \retval 1 succeeded
      \retval 0  nothing happens
    */
    int32_t lowGetAx(URG3D_VSSP_HEADER_T& header,
                     URG3D_AX_HEADER_T& axHeader,
                     URG3D_DATA_AX_T& axData);

    /*!
      \brief get range header in raw format from received data buffer

      \param[out] header  : URG3D_VSSP_HEADER_T struct
      \param[out] axHeader: URG3D_AX_HEADER_T struct
      \param[out] axData  : URG3D_DATA_AX_T sruct

      \retval 1 succeeded
      \retval 0 nothing happens
    */
    int32_t lowGetRangeHeader(URG3D_VSSP_HEADER_T& header,
                              URG3D_RANGE_HEADER_T& rangeHeader,
                              URG3D_RANGE_INDEX_T& rangeIndex);

    std::unique_ptr<Urg3dConnection> m_connection;
    URG3D_CONNECTION_TYPE m_connectionType;
    URG3D_NEXT_PACKET_T m_nextPacket;
    URG3D_TABLES_T m_tables;

    // packet data
    URG3D_VSSP_HEADER_T m_vsspHeader;
    URG3D_RANGE_HEADER_T m_rangeHeader;
    URG3D_RANGE_INDEX_T m_rangeIndex;
    URG3D_DATA_RANGE_T m_rawRange;
    URG3D_DATA_RANGE_INTENSITY_T m_rawRangeIntensity;
    URG3D_AX_HEADER_T m_axHeader;
    URG3D_DATA_AX_T m_rawAuxData;

    // sensor info
    URG3D_SENSOR_INFO_T m_info;

    uint8_t m_isInit;
    uint8_t m_isActive;
    int16_t m_lastErrno;
    uint32_t m_blockingTimeoutMs;
};

#endif
