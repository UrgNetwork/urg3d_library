#ifndef URG_CONNECTION_H
#define URG_CONNECTION_H

/*!
  \file
  \brief functions to handle communication with urg sensor (ethernet)

  \author Satofumi KAMIMURA

  $Id: urg_connection.h,v 1d233c7a2240 2011/02/19 03:08:45 Satofumi $
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "urg_tcpclient.h"


/*!
  \brief constants
*/
enum {
    URG_CONNECTION_TIMEOUT = -1, //!< return value of connection timeout
};


/*!
  \brief communication type
*/
typedef enum {
    URG_ETHERNET,               //!< ethernet
} urg_connection_type_t;


/*!
  \brief communication resource
*/
typedef struct
{
    urg_connection_type_t type; //!< connection type
    urg_tcpclient_t tcpclient;  //!< ethernet
} urg_connection_t;


/*!
  \brief function to connect sensor

  \param[in,out] connection communication resource
  \param[in] connection_type connection type
  \param[in] device device file name or ip address
  \param[in] baudrate_or_port baudrate or port number

  \retval 0 success
  \retval <0 error

  Example (ethernet communication)
  \code
  connection_t connection;
  if (! connection_open(&connection, URG_ETHERNET, "192.168.0.10", 10940)) {
      return 1;
  } \endcode

  \see connection_close()
*/
extern int connection_open(urg_connection_t *connection,
                           urg_connection_type_t connection_type,
                           const char *device, long baudrate_or_port);


/*!
  \brief function to disconnect sensor

  \param[in,out] connection communication resource

  \code
  connection_close(&connection); \endcode

  \see connection_open()
*/
extern void connection_close(urg_connection_t *connection);


/*!
  \brief function to set baudrate
*/
extern int connection_set_baudrate(urg_connection_t *connection, long baudrate);


/*!
  \brief function to send data

  \param[in,out] connection communication resource
  \param[in] data sending data
  \param[in] size byte length of sending data

  \retval >=0 the number of characters sent (on success)
  \retval <0 error code

  Example
  \code
  n = connection_write(&connection, "QT\n", 3); \endcode

  \see connection_read(), connection_readline()
*/
extern int connection_write(urg_connection_t *connection,
                            const char *data, int size);


/*!
  \brief function to receive data

  \param[in,out] connection communication resource
  \param[in] data buffer for received data
  \param[in] max_size byte length of receive buffer
  \param[in] timeout timeout [msec]

  \retval >=0 the number of characters received (on success)
  \retval <0 error code (if no characters are received, this function returns URG_CONNECTION_TIMEOUT)

  If a negative value is set as "timeout", this function waits for receiving data.

  Example
  \code
enum {
    BUFFER_SIZE = 256,
    TIMEOUT_MSEC = 1000,
};
char buffer[BUFFER_SIZE];
n = connection_read(&connection, buffer, BUFFER_SIZE, TIMEOUT_MSEC); \endcode

  \see connection_write(), connection_readline()
*/
extern int connection_read(urg_connection_t *connection,
                           char *data, int max_size, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* !URG_CONNECTION_H */
