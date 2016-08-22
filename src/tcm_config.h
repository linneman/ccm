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

#ifndef TCM_CONFIG_H
#define TCM_CONFIG_H

#include <intercom/server.h>
#include <olcutils/refstring.h>
#include <olcutils/hashmap.h>
#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
    \file tcm_config.h
    \brief server configuration

    \addtogroup config
    @{
 */

/*! maximum length for server addresses according to posix standards */
#define TCM_MAX_ADDR_LEN      108


/*! maximum size of configuration file */
#define TCM_MAX_CONF_SIZE     4096


/*!
 * served REPL IP address
 */
extern char g_tcm_scheme_ip_address[TCM_MAX_ADDR_LEN];


/*!
 * served REPL IP port
 */
extern int  g_tcm_scheme_ip_port;


/*!
 * initialize configuration data
 */
int tcm_init_config(void);


/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* #ifndef TCM_CONFIG_H */
