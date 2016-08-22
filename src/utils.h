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

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#include <olcutils/hashmap.h>
#include <olcutils/lambda.h>
#include <intercom/events.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
    \file utils.h
    \brief utility functions

    \addtogroup utils
    @{
 */


/*!
 * ssc_strlcpy -- size-bounded string copying and concatenation
 *
 * for more detailed information refer to: bsd man page strlcpy
 *
 * taken from:
 *   http://stackoverflow.com/questions/2933725/my-version-of-strlcpy
 *
 * \param dest destination where to store the copied bytes in
 * \param src source where bytes are copied from
 * \param len max length in bytes (octetts) include '\0' termination
 * \return number of copied bytes
 */
size_t tcm_strlcpy(char *dest, const char *src, size_t len);


/*!
 * return pointer to file name extension from null terminated c-string
 *
 * \param filename null terminated c-string with filename
 * \return pointer to file extension of pointer to an empty string if not found
 */
const char *get_filename_ext(const char *filename);


/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef UTILS_H */
