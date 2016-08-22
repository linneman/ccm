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
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <tcm_segfaulthandler.h>
#include <tcm_log.h>


static void tcm_print_trace( void )
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);

  tcm_error ("Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
    tcm_error ("%s\n", strings[i]);

  free (strings);
}

void segfaulthandler( int sig_num )
{

  tcm_error( "%s: !!!!! RECEIVED SEGMENTATION FAULT !!!!!\n", __func__ );
  tcm_error( "=======================================\n" );
  tcm_print_trace();

  exit( -1 );
}

int tcm_enforce_crash( int reccount )
{
  int result;

  if( ! reccount ) {
    int *p = NULL;

    tcm_message("-- %s: make it crash now ...\n", __func__ );
    *p = 42;

    return result;

  } else {
    result = reccount;

    tcm_message("-- %s: enter recursion level %d ...\n", __func__, result );

    return tcm_enforce_crash( --reccount );
  }
}
