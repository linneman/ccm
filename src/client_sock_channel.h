/*
    Asynchronous Communication Channels for Tinyscheme

    The original motivation for the development of this scheme extension was the
    processing of the Hayes AT command set  as used in USB based Wireless Mobile
    Communication Devices  (USB CDC-TCM).  Since we believe  that there  is much
    broader  scope  of  potential  applications, the  implementation  should  be
    considered as a general design pattern.

    Copyright 2016 Otto Linnemann

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see
    <http://www.gnu.org/licenses/>.
*/

#ifndef TCM_CLIENT_SOCK_CHANNEL_H
#define TCM_CLIENT_SOCK_CHANNEL_H

#include <intercom/client.h>
#include <base_channel.h>
#include <tcm_server.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
    \file client_sock_channel.h
    \brief channel for reading and writing from and to TCP/UDP client socket

    \addtogroup channels
    @{
 */

#define CLIENT_SOCK_CH_MAX_DATA_SIZE       256          /*!< maximum data chunk size to be read at once */
#define CLIENT_SOCK_CH_POOL_SIZE           10           /*!< number of data chunks in ring buffer */

/*!
 * device channel object
 */
typedef struct s_client_sock_channel {

  t_base_channel                base;                   /*!< base class */
  t_icom_addr_decl              addr_decl;              /*!< IP or UDP client socket address */
  t_icom_client_conn_handler*   handler;                /*!< client socket handler */

} t_client_sock_channel;


/*!
 * constructor for client socket channel
 *
 * Creates channel queue, reader and processing thread to read and write via TCP/UDP client socket
 * The processing thread invokes a call back function for each element in the channel queue
 *
 * \param p_tcm_server_ctx pointer to main instance object
 * \param addr pointer to C string with ASCII representation of TCP or UDP client address to connect to
 * \param port TCP port or 0 in case of UDP
 * \param p_read_cb callback handler which is invoked by the processing thread
 * \return pointer to channel instance or NULL in case of error
 */
t_client_sock_channel* init_client_sock_channel(
  t_tcm_server_ctx* p_tcm_server_ctx, const char* addr, int port, t_channel_cb p_read_cb );


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef TCM_CLIENT_SOCK_CHANNEL_H */
