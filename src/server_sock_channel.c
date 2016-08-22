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
#include <server_sock_channel.h>
#include <olcutils/alloc.h>
#include <tcm_log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


static int is_server_sock_channel_open( t_base_channel* p_base_channel )
{
  t_server_sock_channel* p = (t_server_sock_channel *)p_base_channel;
  t_icom_sock_listener* p_icom_sock_listener;
  int retcode = 0;

  pthread_mutex_lock( & p->handler->p_connections->mutex );
  p_icom_sock_listener = slist_first( p->handler->p_connections->p_lst_listeners );
  if( p_icom_sock_listener ) {
    retcode = slist_cnt( p_icom_sock_listener->server_list );
  }
  pthread_mutex_unlock( & p->handler->p_connections->mutex );

  return retcode;
}


static int write_server_sock_channel( t_base_channel* p_base_channel, const void* p_arg, const int len )
{
  t_server_sock_channel* p = (t_server_sock_channel *)p_base_channel;
  int retcode = -1;

  if( is_server_sock_channel_open( p_base_channel ) ) {
    retcode = icom_broadcast_to_all( p->handler->p_connections, (char *)p_arg, len );
    if( retcode == 0 ) {
      /* no error */
      retcode = len;
    } else {
      retcode = -1;
    }
  }

  return retcode;
}


static int release_server_sock_channel( t_base_channel* p_base_channel )
{
  t_server_sock_channel* p = (t_server_sock_channel *)p_base_channel;
  int retcode = 0;

  if( p ) {
    icom_kill_server_handlers( p->handler );
    cul_free( p );
  }

  return retcode;
}


t_server_sock_channel* init_server_sock_channel( t_tcm_server_ctx* p_tcm_server_ctx, const char* addr, int port, t_channel_cb p_read_cb )
{
  t_server_sock_channel* p;
  t_base_channel* p_base;
  int retcode;

  p = cul_malloc( sizeof( t_server_sock_channel ) );
  if( p == NULL ) {
    tcm_error( "%s: out of memory error!\n", __func__ );
    return NULL;
  }
  p_base = ((t_base_channel *)p);

  memset( p, 0, sizeof( t_server_sock_channel ) );
  p_base->p_tcm_server_ctx = p_tcm_server_ctx;
  p_base->type = t_channel_server_sock_type;
  p_base->is_open = is_server_sock_channel_open;
  p_base->read = p_read_cb;
  p_base->write = write_server_sock_channel;
  p_base->release = release_server_sock_channel;

  p->decl_table[0].max_connections = SERVER_SOCK_CH_MAX_CONNECTIONS;
  if( port ) { /* network address */
    p->decl_table[0].addr.sock_family = AF_INET;
    p->decl_table[0].addr.port = port;
  } else { /* unix domain socket address */
    p->decl_table[0].addr.sock_family = AF_UNIX;
  }

  strncpy( p->decl_table[0].addr.address, addr, sizeof(p->decl_table[0].addr.address) );

  p->handler = icom_create_server_handlers(
    p->decl_table,
    1,
    SERVER_SOCK_CH_MAX_DATA_SIZE,
    SERVER_SOCK_CH_POOL_SIZE,
    p_read_cb,
    p );

  if( p->handler == NULL ) {
    tcm_error( "%s: could not create server handler error!\n", __func__ );
    cul_free( p );
    return NULL;
  }

  return p;
}
