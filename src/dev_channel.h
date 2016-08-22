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

#ifndef TCM_DEV_CHANNEL_H
#define TCM_DEV_CHANNEL_H

#include <intercom/events.h>
#include <base_channel.h>
#include <tcm_server.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
    \file dev_channel.h
    \brief channel for reading and writing from and to devices and files

    \addtogroup channels
    @{
 */

#define DEV_CH_MAX_DATA_SIZE       256                  /*!< maximum data chunk size to be read at once */
#define DEV_CH_POOL_SIZE           10                   /*!< number of data chunks in ring buffer */
#define DEV_CH_REOPEN_TIME_MS      1000                 /*!< number of milliseconds timeout before retry opening file */

/*!
 * device channel object
 */
typedef struct s_dev_channel {

  t_base_channel                base;                   /*!< base class */

  char                          name[256];              /*!< device / file name */
  int                           fd;                     /*!< device / file descriptor */
  pthread_t                     p_read_handler;         /*!< device read handler */
  t_icom_events*                p_icom_events;          /*!< device I/O handler */
  t_icom_evt*                   p_evt;                  /*!< next processed event */
} t_dev_channel;



/*!
 * constructor for device channel
 *
 * Creates channel queue, reader and processing thread to read and write via to a file or I/O device
 * The processing thread invokes a call back function for each element in the channel queue
 *
 * \param p_tcm_server_ctx pointer to main instance object
 * \param filename full qualified device or pty file name to be accessed
 * \param p_read_cb callback handler which is invoked by the processing thread
 * \return pointer to channel instance or NULL in case of error
 */
t_dev_channel* init_dev_channel( t_tcm_server_ctx* p_tcm_server_ctx, const char* filename, t_channel_cb p_read_cb );


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef TCM_DEV_CHANNEL_H */
