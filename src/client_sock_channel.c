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
#include <client_sock_channel.h>
#include <olcutils/alloc.h>
#include <tcm_log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


static int is_client_sock_channel_open( t_base_channel* p_base_channel )
{
  t_client_sock_channel* p = (t_client_sock_channel *)p_base_channel;
  return( p->handler->connection_state == ICOM_CLIENT_CON_CONNECTED );
}


static int write_client_sock_channel( t_base_channel* p_base_channel, const void* p_arg, const int len )
{
  t_client_sock_channel* p = (t_client_sock_channel *)p_base_channel;
  int retcode = -1;

  if( is_client_sock_channel_open( p_base_channel ) ) {
    retcode = icom_client_handler_request( p->handler, (char *)p_arg, len );
    if( retcode == 0 ) {
      /* no error */
      retcode = len;
    } else {
      retcode = -1;
    }
  }

  return retcode;
}


static int release_client_sock_channel( t_base_channel* p_base_channel )
{
  t_client_sock_channel* p = (t_client_sock_channel *)p_base_channel;
  int retcode = 0;

  if( p ) {
    icom_kill_client_connection_handler( p->handler );
    cul_free( p );
  }

  return retcode;
}


t_client_sock_channel* init_client_sock_channel( t_tcm_server_ctx* p_tcm_server_ctx, const char* addr, int port, t_channel_cb p_read_cb )
{
  t_client_sock_channel* p;
  t_base_channel* p_base;
  int retcode;

  p = cul_malloc( sizeof( t_client_sock_channel ) );
  if( p == NULL ) {
    tcm_error( "%s: out of memory error!\n", __func__ );
    return NULL;
  }
  p_base = ((t_base_channel *)p);

  memset( p, 0, sizeof( t_client_sock_channel ) );
  p_base->p_tcm_server_ctx = p_tcm_server_ctx;
  p_base->type = t_channel_client_sock_type;
  p_base->is_open = is_client_sock_channel_open;
  p_base->read = p_read_cb;
  p_base->write = write_client_sock_channel;
  p_base->release = release_client_sock_channel;

  if( port ) { /* network address */
    p->addr_decl.sock_family = AF_INET;
    p->addr_decl.port = port;
  } else { /* unix domain socket address */
    p->addr_decl.sock_family = AF_UNIX;
  }

  strncpy( p->addr_decl.address, addr, sizeof(p->addr_decl.address) );

  p->handler = icom_create_client_connection_handler(
    & p->addr_decl,
    CLIENT_SOCK_CH_MAX_DATA_SIZE,
    CLIENT_SOCK_CH_POOL_SIZE,
    p_read_cb,
    p );

  if( p->handler == NULL ) {
    tcm_error( "%s: could not create client handler error!\n", __func__ );
    cul_free( p );
    return NULL;
  }

  return p;
}
