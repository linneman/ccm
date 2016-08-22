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

#ifndef TCM_BASE_CHANNEL_H
#define TCM_BASE_CHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tcm_server.h>
#include <tinyscheme/scheme.h>


/*!
    \file base_channel.h
    \brief channel base class

    \addtogroup channels
    @{
 */

struct s_base_channel;

/*!
 *  request handler type 0
 *
 *  \param p pointer to channel instance
 *  \return 0 in case of success, otherwise negative error code
 */
typedef int (*t_channel_handler_0) ( struct s_base_channel* p );

/*!
 *  request handler type 1
 *
 *  \param p pointer to channel instance
 *  \param pointer to first argument
 *  \return intereger return code according to handler type
 */
typedef int (*t_channel_handler_1) ( struct s_base_channel* p, const void* p_arg );


/*!
 *  request handler type 2
 *
 *  \param p pointer to channel instance
 *  \param pointer to first argument (buffer)
 *  \param buffer size
 *  \return intereger return code according to handler type
 */
typedef int (*t_channel_handler_2) ( struct s_base_channel* p, const void* p_arg, const int len );


/*!
 *  event callback handler to receive data from channel
 *
 *  refer to type t_evt_cb in libintercom
 */
typedef t_evt_cb  t_channel_cb;


/*!
 *  type of channel
 */
typedef enum {
  t_channel_dev_type,                                   /*!< file respectively device type */
  t_channel_client_sock_type,                           /*!< TCP or UDP client socket type */
  t_channel_server_sock_type                            /*!< TCP or UDP server socket type */
} t_channel_type;


/*!
 * abstract channel base object
 *
 * The concept  is that derived objects  handle a background thread  for reading
 * which is created during initialization.  The background thread invokes a call
 * back  function whenever  data is  read. Thus  only handlers  for writing  and
 * releasing have to be invoked explicitly.
 */
typedef struct s_base_channel {

  t_tcm_server_ctx*             p_tcm_server_ctx;       /*!< back reference to server context */
  t_channel_type                type;                   /*!< type of channel */

  t_channel_handler_0           is_open;                /*!< return 1 when open, otherwise 0 */
  t_channel_handler_0           release;                /*!< close channel and processing thread */
  t_channel_handler_2           write;                  /*!< write handler */
  t_channel_cb                  read;                   /*!< read callback function, invoked from thread context */

  char                          cb_symbol_name[256];    /*!< scheme callback function symbol name */
  pointer                       p_cb_closure_code;      /*!< scheme callback closure to invoked */

} t_base_channel;


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef TCM_BASE_CHANNEL_H */
