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
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <unistd.h>

#include <olcutils/alloc.h>
#include <olcutils/memtrace.h>
#include <olcutils/revision.h>
#include <intercom/log.h>
#include <intercom/revision.h>

#include <tcm_server.h>
#include <utils.h>
#include <revision.h>
#include <common.h>
#include <tcm_segfaulthandler.h>
#include <tcm_scheme.h>
#include <tcm_config.h>
#include <tcm_log.h>

#include <dev_channel.h>


static void tcm_release( t_tcm_server_ctx* p )
{
  if( ! p )
    return;

  tcm_release_scheme( p->p_scheme );
  tcm_message("\tscheme interpreter killed\n" );

  cul_free( p );
}


static t_tcm_server_ctx* tcm_init( void )
{
  t_tcm_server_ctx* p;

  p = cul_malloc( sizeof( t_tcm_server_ctx ) );
  if( ! p )
    return NULL;

  memset( p, 0, sizeof(t_tcm_server_ctx) );

  p->p_scheme = tcm_init_scheme( p );
  if( p->p_scheme == NULL ) {
    tcm_error( "could not start scheme interpreter, exit daemon!\n" );
    tcm_release( p );
    return NULL;
  }

  return p;
}

int read_test( t_icom_evt* p_evt )
{
  printf("%s: received: %s\n", __func__, (char *) p_evt->p_data );

  return 0;
}

int tcm(void)
{
  cul_allocstat_t allocstat;
  t_tcm_server_ctx* p;
  int result = 0;
  long run_loop_cnt = 1;
  t_dev_channel* p_dev_channel = NULL;

  memtrace_enable();

  tcm_log_init();
  tcm_message("TCM Daemon started\n" );
  tcm_message("tcm daemon revision: %s\n", g_tcm_revision );
  tcm_message("libcutils revision: %s\n", g_cutillib_revision );
  tcm_message("libintercom revision: %s\n", g_icomlib_revision );
  tcm_init_config();

  p =  tcm_init();
  if( ! p ) {
    tcm_error( "%s: server initialization error!\n" );
    return -1;
  }


  /* test case for crashdump */
  /* tcm_enforce_crash( 10 ); */

  /* main process loop waits just for termination */
  while( ! p->termination_request ) {
    sleep( 1 );
    if( ! (run_loop_cnt % 10) )
      tcm_message("alive\n");
    ++run_loop_cnt;


    if( run_loop_cnt == 200000 )
    {
      printf("%s: open test channel ...\n", __func__ );
      p_dev_channel = init_dev_channel( p, "/tmp/test", read_test );

      if( p_dev_channel ) {
        printf( "%s: server startet\n", __func__ );
      }
    }
  }

  if( p_dev_channel )
    ((t_base_channel *)p_dev_channel)->release( (t_base_channel *)p_dev_channel );

  tcm_release( p );

  memtrace_disable();

  allocstat = get_allocstat();
  printf("\n\n");
  printf("Memory Allocation Statistics in cutillib functions\n");
  printf("--------------------------------------------------\n");
  printf("       number of open allocations: %ld\n", allocstat.nr_allocs );
  printf("     number of failed allocations: %ld\n", allocstat.nr_allocs_failed );
  printf("  still allocated memory in bytes: %ld\n", allocstat.mem_allocated );
  printf("maximum allocated memory in bytes: %ld\n", allocstat.max_allocated );
  printf("\n");

  if( allocstat.nr_allocs != 0 || allocstat.mem_allocated != 0 )
  {
    fprintf( stderr, "ATTENTION: Some memory is leaking out, configure with memcheck option and check for root cause!\n\n");
    result = -1;
  }

  // memtrace_print_log( stdout );

  tcm_log_release();
  return result;
}


int main( int argc, char* argv[] )
{
  if( signal( SIGSEGV, segfaulthandler ) == SIG_ERR )
    tcm_error( "Could not register SIGSEGV error!\n");

  if( signal( SIGFPE, segfaulthandler ) == SIG_ERR )
    tcm_error( "Could not register SIGFPE error!\n");

  if( signal( SIGPIPE, segfaulthandler ) == SIG_ERR )
    tcm_error( "Could not register SIGPIPE error!\n");

  return tcm();
}
