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
#include <string.h>
#include <dev_channel.h>
#include <olcutils/alloc.h>
#include <tcm_log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


static void* dev_read_handler( void* pCtx )
{
  t_dev_channel* p = (t_dev_channel *)pCtx;
  t_icom_events* p_events = p->p_icom_events;
  long cnt = 0;

  while( 1 )  /* open loop */
  {
    tcm_message( "%s for dev name %s (re)started\n", __func__, p->name );
    p->fd = open( p->name, O_RDWR );
    if( p->fd >= 0 ) {
      /* success */
      tcm_message( "%s: successfully opened %s, start reading form descriptor %d ...\n", __func__, p->name, p->fd );

      while( 1 )   /* read loop */
      {
        /* unlink event for processing out of pool */
        pthread_mutex_lock( & p_events->mutex );
        if ( !IsListEmpty( & p_events->pool ) ) {
          p->p_evt = (t_icom_evt*)RemoveHeadList( & p_events->pool );
        }
        else {
          tcm_error("event queue overflow error, overwriting existing events\n");
          p->p_evt = (t_icom_evt*)RemoveHeadList( & p_events->ready_list );
        }
        pthread_mutex_unlock( & p_events->mutex );

        p->p_evt->type = ICOM_EVT_CLIENT_DATA;
        p->p_evt->p_user_ctx = p;
        memset( p->p_evt->p_data, 0, p->p_evt->max_data_size );

        p->p_evt->data_len = read( p->fd, p->p_evt->p_data, p->p_evt->max_data_size );
        if( p->p_evt->data_len > 0 )
        {
// #define TEST
#ifdef TEST
          tcm_message( "received message: %s\n", p->p_evt->p_data );
#endif /* #ifdef TEST */

          /* insert newly created event in ready list */
          pthread_mutex_lock( & p_events->mutex );
          InsertTailList( & p_events->ready_list, & p->p_evt->node);
          p->p_evt = NULL;
          pthread_cond_signal( & p_events->signal );
          pthread_mutex_unlock( & p_events->mutex );

          ++cnt;
        }
        else
        {
          tcm_error( "%s: file reader stream broke!\n", __func__ );

          /* put unprocessed event back in pool */
          pthread_mutex_lock( & p_events->mutex );
          InsertTailList( & p_events->pool, & p->p_evt->node );
          p->p_evt = NULL;
          pthread_mutex_unlock( & p_events->mutex );

          close( p->fd );
          p->fd = -1;
          break;
        }
      } /* read loop */
    }
    else
    {
      tcm_error( "%s: could not open file %s, try again in %d milliseconds ...\n", __func__, p->name, DEV_CH_REOPEN_TIME_MS );
      usleep( 1000L * DEV_CH_REOPEN_TIME_MS );
    }
  } /* open loop */

  return p;
}

static int is_dev_channel_open( t_base_channel* p_base_channel )
{
  t_dev_channel* p = (t_dev_channel *)p_base_channel;
  int retcode = 0;

  if( p->fd >= 0 ) {
    retcode = 1;
  }

  return retcode;
}

static int write_dev_channel( t_base_channel* p_base_channel, const void* p_arg, const int len )
{
  t_dev_channel* p = (t_dev_channel *)p_base_channel;
  int retcode = 0;

  if( p->fd >= 0 ) {
    retcode = write( p->fd, (char *)p_arg, len );
  } else {
    tcm_error("%s: could not write to channel %s error!\n", __func__, p->name );
    retcode = -1;
  }

  return retcode;
}

static int release_dev_channel( t_base_channel* p_base_channel )
{
  t_dev_channel* p = (t_dev_channel *)p_base_channel;
  t_icom_events* p_events = p->p_icom_events;
  int retcode = 0;

  if( p )
  {
    if( p->p_read_handler )
      pthread_cancel( p->p_read_handler );

    if( p->fd ) {
      close( p->fd );
      p->fd = -1;
    }

    if( p_events ) {
      if( p->p_evt ) {
        /* put eventually remaining tempory buffer back in pool */
        pthread_mutex_lock( & p_events->mutex );
        InsertTailList( & p_events->pool, & p->p_evt->node );
        pthread_mutex_unlock( & p_events->mutex );
      }
      kill_icom_event_handler( p_events );
    }

    cul_free( p );
  }

  return retcode;
}

t_dev_channel* init_dev_channel( t_tcm_server_ctx* p_tcm_server_ctx, const char* filename, t_channel_cb p_read_cb )
{
  t_dev_channel* p;
  t_base_channel* p_base;
  int retcode;

  p = cul_malloc( sizeof( t_dev_channel ) );
  if( p == NULL ) {
    tcm_error( "%s: out of memory error!\n", __func__ );
    return NULL;
  }
  p_base = ((t_base_channel *)p);

  memset( p, 0, sizeof( t_dev_channel ) );
  p_base->p_tcm_server_ctx = p_tcm_server_ctx;
  p_base->type = t_channel_dev_type;
  p_base->is_open = is_dev_channel_open;
  p_base->read = p_read_cb;
  p_base->write = write_dev_channel;
  p_base->release = release_dev_channel;
  p->fd = -1; /* to indicate non initialized descriptor */

  strncpy( p->name, filename, sizeof( p->name ) );

  p->p_icom_events = icom_create_event_handler( DEV_CH_MAX_DATA_SIZE, DEV_CH_POOL_SIZE, p_read_cb );
  if( p->p_icom_events == NULL  ) {
    tcm_error( "%s: creation of event handler failed\n", __func__ );
    release_dev_channel( p_base );
    p = NULL;
  }

  retcode = pthread_create( &p->p_read_handler, NULL, dev_read_handler, p );
  if( ! retcode )
    retcode = pthread_detach( p->p_read_handler );
  if( retcode ) {
    tcm_error( "%s: creation of read handler thread failed with error %d\n", __func__, retcode );
    release_dev_channel( p_base );
    p = NULL;
  }

  return p;
}
