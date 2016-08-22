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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <utils.h>
#include <common.h>
#include <olcutils/alloc.h>
#include <intercom/server.h>


/*!
 * tcm_strlcpy -- size-bounded string copying and concatenation
 *
 * for more detailed information refer to: bsd man page strlcpy
 *
 * taken from:
 *   http://stackoverflow.com/questions/2933725/my-version-of-strlcpy
 */
size_t tcm_strlcpy(char *dest, const char *src, size_t len)
{
    char *d = dest;
    char *e = dest + len; /* end of destination buffer */
    const char *s = src;

    /* Insert characters into the destination buffer
       until we reach the end of the source string
       or the end of the destination buffer, whichever
       comes first. */
    while (*s != '\0' && d < e)
        *d++ = *s++;

    /* Terminate the destination buffer, being wary of the fact
       that len might be zero. */
    if (d < e)        // If the destination buffer still has room.
        *d = 0;
    else if (len > 0) // We ran out of room, so zero out the last char
                      // (if the destination buffer has any items at all).
        d[-1] = 0;

    /* Advance to the end of the source string. */
    while (*s != '\0')
        s++;

    /* Return the number of characters
       between *src and *s,
       including *src but not including *s .
       This is the length of the source string. */
    return s - src;
}


const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
