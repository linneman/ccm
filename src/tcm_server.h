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

#ifndef TCM_SERVER_H
#define TCM_SERVER_H

#include <intercom/server.h>
#include <olcutils/refstring.h>
#include <olcutils/hashmap.h>
#include <olcutils/slist.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \file tcm_server.h
    \brief served actions

    \addtogroup scheme
    @{
 */


struct s_tcm_scheme;

/*!
 * tcm server respectively daemon state
 *
 * Manages main application data like scheme instance and other state information
 */
typedef struct s_tcm_server_ctx {
  struct s_tcm_scheme*        p_scheme;                 /*!< pointer to scheme instance object */
  int                         termination_request;      /*!< terminate process when set to 1 */
} t_tcm_server_ctx;


/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* #ifndef TCM_SERVER_H */
