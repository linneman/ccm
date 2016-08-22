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

#ifndef SEGFAULT_HANDLER_H
#define SEGFAULT_HANDLER_H

#include <signal.h>
#include <tcm_segfaulthandler.h>


#ifdef __cplusplus
extern "C" {
#endif

/*!
    \file tcm_segfaulthandler.h
    \brief segmentation fault handler (debugging)

    \addtogroup utils
    @{
 */


/*!
 * segmentation fault handler
 * \param sig_num: received signal
 */
void segfaulthandler( int sig_num );


/*!
 * function to enforce crash after specified number of recursions.
 * Do not invoke this!
 *
 * \param reccount number of recursive invocations before crash
 * \return reccount
 *
 */
int tcm_enforce_crash( int reccount );


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* SEGFAULT_HANDLER_H */
