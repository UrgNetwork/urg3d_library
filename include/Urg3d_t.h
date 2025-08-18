#ifndef URG3D_T
#define URG3D_T

#include <stdint.h>
#include <vector>
#include "Urg3dConnection.h"

/*------------- VSSP <= 2.2 ------------*/
const uint8_t  URG3D_MAX_MOTOR_INTERLACE_COUNT = 20;
const uint8_t  URG3D_MAX_REM_INTERLACE_COUNT = 10;
const uint16_t URG3D_FIRST_LINE_NUMBER = 0;
const uint16_t URG3D_LAST_LINE_NUMBER = 35;
const uint16_t URG3D_LINE_COUNT = 36;
const uint16_t URG3D_MAX_SPOTS_COUNT = 74;
const uint8_t  URG3D_MAX_ECHOS_COUNT = 4;
const uint8_t  URG3D_REGION_COUNT = 2;
const uint8_t  URG3D_IS_LSB_RESULT = 0;
const uint16_t URG3D_ACCEL_SCALE = 4;
const uint16_t URG3D_GYRO_SCALE = 500;
const uint8_t  URG3D_SWING_DIR = 0;

/*--------------------------------------*/
const uint8_t  URG3D_STATUS_LENGTH = 3;
const uint8_t  URG3D_TYPE_LENGTH = 3;
const uint8_t  URG3D_MAX_TYPE_COUNT = 32;
const uint8_t  URG3D_MAX_AUX_COUNT = 255;
const uint8_t  URG3D_MAX_STRING_SIZE = 64;
const uint16_t URG3D_MAX_RX_LENGTH = 16384;
const uint8_t  URG3D_MAX_TX_LENGTH = 64;
const uint8_t  URG3D_VSSP_HEADER_LENGTH = 24;
const uint16_t URG3D_VSSP_SPOTS_IN_GROUP = 256;

using namespace std;

struct URG3D_NEXT_PACKET_T {
    uint8_t nextHeaderReady;
    uint8_t nextDataReady;
    char    nextHeader[URG3D_VSSP_HEADER_LENGTH];
};

struct URG3D_TABLES_T {
    vector<double> spotMotorAngleRatio; // size : max spot count
    vector<vector<vector<double>>> spotRemAngleRad; // size : [max rem interlace count][max rem interlace count][max spot count]
    vector<vector<vector<double>>> cosRemAngleRad; // size : [max rem interlace count][max rem interlace count][max spot count]
    vector<vector<vector<double>>> sinRemAngleRad; // size : [max rem interlace count][max rem interlace count][max spot count]
    vector<uint8_t> spotRemAngleLoaded; // size: max rem interlace count
};

struct URG3D_SENSOR_SPEC_T {
    uint8_t motorInterlaceCount;
    uint8_t remInterlaceCount;
    uint16_t firstLineNumber;
    uint16_t lastLineNumber;
    uint16_t lineCount;
    uint16_t spotCount;
    uint8_t  echoCount;
    uint8_t  regionCount;
    uint8_t  isLsbResult;
    uint16_t accelScale;
    uint16_t gyroScale;
    uint8_t  swingDir;
};

struct URG3D_SENSOR_VERSION_T {
    char vendor[URG3D_MAX_STRING_SIZE];
    char product[URG3D_MAX_STRING_SIZE];
    char serial[URG3D_MAX_STRING_SIZE];
    char firmware[URG3D_MAX_STRING_SIZE];
    char protocol[URG3D_MAX_STRING_SIZE];
};

struct URG3D_SENSOR_INFO_T {
    URG3D_SENSOR_SPEC_T spec;
    URG3D_SENSOR_VERSION_T version;
};

struct URG3D_VSSP_HEADER_T {
    char     mark[4];
    char     type[4];
    char     status[4];
    uint16_t headerLength;
    uint16_t length;
    uint32_t receivedTimeMs;
    uint32_t sendTimeMs;
};

struct URG3D_RANGE_HEADER_T {
    // common(ro, ri, ra)
    uint16_t headerLength;
    uint32_t lineHeadTimestamp;
    uint32_t lineTailTimestamp;
    int16_t  lineHeadMotorAngleRatio;
    int16_t  lineTailMotorAngleRatio;
    uint8_t  frame;
    uint8_t  motorField;
    uint8_t  remField;
    uint8_t  remInterlace;
    uint16_t line;
    uint16_t spot;
    // for ra
    uint16_t dustInfo;
    uint8_t  output;
    uint8_t  input;
    uint16_t dirtInfo;
    vector<uint32_t> totalDetectionArea; // size : max region count
};

struct URG3D_RANGE_INDEX_T {
    uint16_t indexLength;
    uint16_t nspots;
    vector<uint16_t> index; // size : max spot count + 1
    // +1 means "uint16_t necho;"
};

struct URG3D_RAW_RANGE_INTENSITY_T {
    uint16_t rangeMm;
    uint16_t intensity;
};

struct URG3D_DATA_RANGE_INTENSITY_T {
    vector<URG3D_RAW_RANGE_INTENSITY_T> raw; // size : (max spot count) * (max echo count)
};

struct URG3D_RAW_RANGE_T {
    uint16_t rangeMm;
};

struct URG3D_DATA_RANGE_T {
    vector<URG3D_RAW_RANGE_T> raw; // size : (max spot count) * (max echo count)
};

struct URG3D_AX_HEADER_T {
    uint16_t headerLength;
    uint32_t timestampMs;
    uint32_t dataBitfield;
    uint8_t  dataCount;
    uint8_t  dataMs;
};

struct URG3D_DATA_AX_T {
    int32_t value[URG3D_MAX_TYPE_COUNT * URG3D_MAX_AUX_COUNT];
};

enum URG3D_MEASUREMENT_TYPE {
    URG3D_NO_REQUEST = 0,                    //No data available
    URG3D_DISTANCE = 1,                      //ro
    URG3D_DISTANCE_INTENSITY = 2,            //ri
    URG3D_AUXILIARY = 3,                     //ax
    URG3D_AUGMENTED_DISTANCE_INTENSITY = 4,  //ra
};

struct URG3D_POLAR_T {
    double   rangeM, verticalRad, horizontalRad;
    uint32_t intensity;
};

struct URG3D_POINT_T {
    double xm, ym, zm;
    uint32_t intensity;
};

struct URG3D_LN_RESULT_T {
    vector<uint8_t>  isDetected; // size : max region count
};

struct URG3D_SPOT_T {
    vector<URG3D_POLAR_T> polar; // size : max echo count
    vector<URG3D_POINT_T> point; // size : max echo count
    vector<URG3D_LN_RESULT_T> detectionResult; // size : max echo count
    uint32_t echoCount;
};

struct URG3D_MEASUREMENT_DATA_T {
    vector<URG3D_SPOT_T> spots; // size : max spot count
    uint32_t timestamp;
    uint8_t  frameNumber;
    uint8_t  motorFieldNumber;
    uint8_t  remFieldNumber;
    uint16_t lineNumber;
    uint16_t spotCount;
    char     status[URG3D_STATUS_LENGTH]; //without LF
    uint8_t  dustState;
    uint16_t dustRatioX10;
    uint8_t  output;
    uint8_t  input;
    uint8_t  dirtState;
    uint16_t dirtRatioX10;
    vector<uint32_t> totalDetectionArea; // size : max region count

    uint8_t  isLineDataReady;
};

struct URG3D_AUXILIARY_RECORD_T {
    uint32_t timestampMs;
    int32_t  gyroX, gyroY, gyroZ;
    int32_t  accelX, accelY, accelZ;
    int32_t  compassX, compassY, compassZ;
    int32_t  temperature;
};

enum URG3D_AUXILIARY_TYPE {
    URG3D_NO_RECORD = (0),  //No data available
    URG3D_GYRO_DATA = (1 << 0),
    URG3D_ACCEL_DATA = (1 << 1),
    URG3D_COMPASS_DATA = (1 << 2),
    URG3D_TEMPERATURE_DATA = (1 << 3),
};

struct URG3D_AUXILIARY_DATA_T {
    uint32_t timestampMs;
    URG3D_AUXILIARY_RECORD_T records[URG3D_MAX_AUX_COUNT];
    URG3D_AUXILIARY_TYPE     type;
    uint32_t recordCount;
    char     status[3]; //without LF
};

#endif // URG3D_T
