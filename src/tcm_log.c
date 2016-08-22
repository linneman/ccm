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
#include <time.h>
#include <sys/syslog.h>

#include <tcm_log.h>

#define LOG_SETTING  ( LOG_NOWAIT | LOG_PID )

/*! initialize logging (syslog) */
int tcm_log_init(void)
{
  openlog( LOG_TAG, LOG_SETTING, LOG_SYSLOG );

  return 0;
}


/*! release logging (syslog) */
void tcm_log_release(void)
{
  closelog();
}


/*!
 * for message log output, currently to stdout
 */
void tcm_message( const char* fmt, ... )
{
  va_list args;

  va_start( args, fmt );
  vsyslog( LOG_NOTICE, fmt, args );
  va_end( args );
}

/*!
 * for error log output, currently to stderr
 */
void tcm_error( const char* fmt, ... )
{
  va_list args;

  va_start( args, fmt );
  /* LOG_WARNING used temporarily since its enabled by default on
     debian's rsyslog daemon configuration */
  vsyslog( LOG_WARNING, fmt, args );
  va_end( args );
}
