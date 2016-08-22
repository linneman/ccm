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
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <getopt.h>

#include <olcutils/alloc.h>
#include <tinyscheme/dynload.h>
#include <tcm_scheme.h>
#include <tcm_config.h>
#include <tcm_log.h>
#include <dev_channel.h>
#include <client_sock_channel.h>
#include <server_sock_channel.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? a : b) /*!< minimum function \param a 1st arg, \param b 2nd arg */
#endif

/*!
    \file tcm_scheme_ext.c
    \brief native tcm extensions for the embedded scheme script interpreter

    \addtogroup scheme
    @{
 */


/*!
 * invoke system shell with given command
 *  this is temporarily used only
 *
 * try:
 * (system "touch /etc/hello")
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument lis t
 * \return pointer to scheme boolean value, T when operation was successful
 */
static pointer scm_system(scheme *sc, pointer args)
{
  pointer arg;
  pointer retval;
  char    *strarg;
  double  realnumber;
  int     intnumber;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;

  while( args != sc->NIL )
  {
    if( i > 0 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes only one argument error!\n" );
      errors = -1;
    }
    else if( is_string( arg = pair_car(args)) ) {
      strarg = string_value( arg );
    }
    else if( is_real( arg = pair_car(args) ) ) {
      snprintf( outbuf, sizeof(outbuf), "wrong argument type, must be string!\n" );
      errors = -1;
      break;
    }
    else if( is_integer( arg = pair_car(args) ) ) {
      snprintf( outbuf, sizeof(outbuf), "wrong argument type, must be string!\n" );
      errors = -1;
      break;
    }

    args = pair_cdr( args );
    ++i;
  }

  if( ! errors ) {
    errors =  system( strarg );
    if( errors ) {
      snprintf( outbuf, sizeof(outbuf), "execution error!\n" );
    } else {
      snprintf( outbuf, sizeof(outbuf), "OK\n" );
    }
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  if( errors ) {
    tcm_error( "%s: %s", __func__, outbuf );
    retval = sc -> F;
  } else {
    tcm_message( "%s: %s successfully executed\n", __func__, strarg );
    retval = sc -> T;
  }

  return(retval);
}

/*!
 * pause execution for x seconds (blocking)
 *
 * try:
 * (sleep 1)
 * (sleep 1.5)
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument lis t
 * \return pointer to scheme boolean value, T when operation was successful
 */
static pointer scm_sleep(scheme *sc, pointer args)
{
  pointer arg;
  pointer retval;
  char    *strarg;
  double  realnumber;
  int     intnumber;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  useconds_t usec;

  while( args != sc->NIL )
  {
    if( i > 0 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes only one argument error!\n" );
      errors = -1;
      break;
    }
    else if( is_string( arg = pair_car(args)) ) {
      snprintf( outbuf, sizeof(outbuf), "wrong argument type, must be integer or real number!\n" );
      errors = -1;
      break;
    }
    else if( is_real( arg = pair_car(args) ) ) {
      realnumber = rvalue( arg );
      usec= (useconds_t) (realnumber * 1e6);
    }
    else if( is_integer( arg = pair_car(args) ) ) {
      intnumber = ivalue( arg );
      usec= (useconds_t) ((long)intnumber * 1000000L);
    }

    args = pair_cdr( args );
    ++i;
  }

  if( ! errors ) {
    tcm_message( "%s: blocking delay for %ld microseconds ...\n", __func__, usec );
    errors =  usleep( usec );
    tcm_message( "%s: done\n", __func__ );
    if( errors ) {
      snprintf( outbuf, sizeof(outbuf), "execution error!\n" );
    } else {
      snprintf( outbuf, sizeof(outbuf), "OK\n" );
    }
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  if( errors ) {
    tcm_error( "%s: %s", __func__, outbuf );
    retval = sc -> F;
  } else {
    retval = sc -> T;
  }

  return(retval);
}


/*!
 * quit daemon (required for proper debugging)
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument lis t
 * \return pointer to scheme true value
 */
static pointer scm_quit(scheme *sc, pointer args)
{
  pointer retval;
  t_tcm_scheme* p_tcm_sceme = (t_tcm_scheme *)sc;

  p_tcm_sceme->p_tcm_server_ctx->termination_request = 1;

  retval = sc -> T;

  return(retval);
}

/*!
 * wraps IPC callback to scheme callback function
 *
 * p_evt pointer to libintercom event object
 */
static int read_cb_wrapper( t_icom_evt* p_evt )
{
  t_dev_channel* p = (t_dev_channel *)p_evt->p_user_ctx;
  t_base_channel* p_base = (t_base_channel *)p;
  t_tcm_scheme*  p_scheme = p_base->p_tcm_server_ctx->p_scheme;
  scheme* sc = (scheme *) p_scheme;
  char    cb_symbol_name[80] = { '\0' };
  pointer retval;

  if( p_evt->type == ICOM_EVT_SERVER_DATA || p_evt->type == ICOM_EVT_CLIENT_DATA )
  {
    tcm_message("%s: received: %.30s\n", __func__, (char *) p_evt->p_data );

    snprintf( cb_symbol_name, sizeof(cb_symbol_name), "dev-ch-cb-%s", p->name );

    pthread_mutex_lock( & p_scheme->mutex );
    retval = scheme_call( sc, p_base->p_cb_closure_code, cons( sc, mk_string( sc, p_evt->p_data ), sc->NIL ) );
    pthread_mutex_unlock( & p_scheme->mutex );
  }

  return 0;
}

/*!
 * create a new device channel
 *
 * try: (make-dev-channel "/tmp/test")
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument list
 * \return pointer to channel identifier
 */
static pointer scm_make_dev_channel(scheme *sc, pointer args)
{
  t_tcm_scheme* p_tcm_scheme = (t_tcm_scheme *)sc;
  pointer arg;
  pointer retval;
  char    *filename;
  double  realnumber;
  int     intnumber;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  pointer closure_code;
  // pointer closure_env;
  t_dev_channel* p_dev_channel;
  t_base_channel* p_base_channel;

  while( args != sc->NIL )
  {
    if( i > 1 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes two arguments only error!\n" );
      errors = -1;
      break;
    }
    else if( i == 0  ) {
      if( is_string( arg = pair_car(args)) ) {
        filename = string_value( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "first argument must be file name string!\n" );
        errors = -1;
        break;
      }
    }
    else if( i == 1 ) {
      if( is_closure( arg = pair_car(args) ) ) {
        closure_code = arg;
        // closure_env = sc->vptr->closure_env( arg );
        // printf("code flag: %d\n", closure_code->_flag );
        // printf("env flag: %d\n", closure_env->_flag );
      } else {
        snprintf( outbuf, sizeof(outbuf), "second argument must be call back function!\n" );
        errors = -1;
        break;
      }
    }

    args = pair_cdr( args );
    ++i;
  }

  if( i == 2 ) {
    if( ! errors ) {
      p_dev_channel = init_dev_channel( p_tcm_scheme->p_tcm_server_ctx, filename, read_cb_wrapper );
      if( p_dev_channel ) {
        p_base_channel = (t_base_channel *) p_dev_channel;
        p_base_channel->p_cb_closure_code = closure_code;

        /* link symbol to callback closure to avoid gc to clean it up */
        snprintf( p_base_channel->cb_symbol_name, sizeof(p_base_channel->cb_symbol_name), "dev-ch-cb-%s", filename );
        scheme_define( sc, sc->global_env, mk_symbol( sc, p_base_channel->cb_symbol_name ), closure_code );
        sprintf( outbuf, "ok\n" );
      } else {
        sprintf( outbuf, "could not create device channel error\n" );
        errors = -1;
      }
    }
  } else {
    sprintf( outbuf, "function takes two arguments (device name, callback) error\n" );
    errors = -1;
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  if( errors ) {
    tcm_error( "%s: %s", __func__, outbuf );
    retval = sc -> F;
  } else {
    retval = mk_integer( sc, (long)p_dev_channel );
  }

  return(retval);
}


/*!
 * create a new client socket channel
 *
 * try: (make-client-sock-channel "127.0.0.1" 5000)
 *     (make-client-sock-channel "/tmp/test.sock" 0)
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument list
 * \return pointer to channel identifier
 */
static pointer scm_make_client_sock_channel(scheme *sc, pointer args)
{
  t_tcm_scheme* p_tcm_scheme = (t_tcm_scheme *)sc;
  pointer arg;
  pointer retval;
  char    *addr;
  int     port;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  pointer closure_code;
  t_client_sock_channel* p_client_sock_channel;
  t_base_channel* p_base_channel;

  while( args != sc->NIL )
  {
    if( i > 2 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes three arguments only error!\n" );
      errors = -1;
      break;
    }
    else if( i == 0  ) {
      if( is_string( arg = pair_car(args)) ) {
        addr = string_value( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "first argument must be string with IP or UDS address!\n" );
        errors = -1;
        break;
      }
    }
    else if( i == 1  ) {
      if( is_integer( arg = pair_car(args)) ) {
        port = ivalue( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "second argument must be port or 0 for UDS address!\n" );
        errors = -1;
        break;
      }
    }
    else if( i == 2 ) {
      if( is_closure( arg = pair_car(args) ) ) {
        closure_code = arg;
      } else {
        snprintf( outbuf, sizeof(outbuf), "third argument must be call back function!\n" );
        errors = -1;
        break;
      }
    }

    args = pair_cdr( args );
    ++i;
  }

  if( i == 3 ) {
    if( ! errors ) {
      p_client_sock_channel = init_client_sock_channel( p_tcm_scheme->p_tcm_server_ctx, addr, port, read_cb_wrapper );
      if( p_client_sock_channel ) {
        p_base_channel = (t_base_channel *) p_client_sock_channel;
        p_base_channel->p_cb_closure_code = closure_code;

        /* link symbol to callback closure to avoid gc to clean it up */
        snprintf( p_base_channel->cb_symbol_name, sizeof(p_base_channel->cb_symbol_name), "client-sock-ch-cb-%s-%d", addr, port );
        scheme_define( sc, sc->global_env, mk_symbol( sc, p_base_channel->cb_symbol_name ), closure_code );
        sprintf( outbuf, "ok\n" );
      } else {
        sprintf( outbuf, "could not create device channel error\n" );
        errors = -1;
      }
    }
  } else {
    sprintf( outbuf, "function takes three arguments (addr, port, callback) error!\n" );
    errors = -1;
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  if( errors ) {
    tcm_error( "%s: %s", __func__, outbuf );
    retval = sc -> F;
  } else {
    retval = mk_integer( sc, (long)p_client_sock_channel );
  }

  return(retval);
}

/*!
 * create a new server socket channel
 *
 * try: (make-server-sock-channel "127.0.0.1" 5000)
 *      (make-server-sock-channel "/tmp/test.sock" 0)
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument lis t
 * \return pointer to channel identifier
 */
static pointer scm_make_server_sock_channel(scheme *sc, pointer args)
{
  t_tcm_scheme* p_tcm_scheme = (t_tcm_scheme *)sc;
  pointer arg;
  pointer retval;
  char    *addr;
  int     port;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  pointer closure_code;
  t_server_sock_channel* p_server_sock_channel;
  t_base_channel* p_base_channel;

  while( args != sc->NIL )
  {
    if( i > 2 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes three arguments only error!\n" );
      errors = -1;
      break;
    }
    else if( i == 0  ) {
      if( is_string( arg = pair_car(args)) ) {
        addr = string_value( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "first argument must be string with IP or UDS address!\n" );
        errors = -1;
        break;
      }
    }
    else if( i == 1  ) {
      if( is_integer( arg = pair_car(args)) ) {
        port = ivalue( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "second argument must be port or 0 for UDS address!\n" );
        errors = -1;
        break;
      }
    }
    else if( i == 2 ) {
      if( is_closure( arg = pair_car(args) ) ) {
        closure_code = arg;
      } else {
        snprintf( outbuf, sizeof(outbuf), "third argument must be call back function!\n" );
        errors = -1;
        break;
      }
    }

    args = pair_cdr( args );
    ++i;
  }

  if( i == 3 ) {
    if( ! errors ) {
      p_server_sock_channel = init_server_sock_channel( p_tcm_scheme->p_tcm_server_ctx, addr, port, read_cb_wrapper );
      if( p_server_sock_channel ) {
        p_base_channel = (t_base_channel *) p_server_sock_channel;
        p_base_channel->p_cb_closure_code = closure_code;

        /* link symbol to callback closure to avoid gc to clean it up */
        snprintf( p_base_channel->cb_symbol_name, sizeof(p_base_channel->cb_symbol_name), "client-sock-ch-cb-%s-%d", addr, port );
        scheme_define( sc, sc->global_env, mk_symbol( sc, p_base_channel->cb_symbol_name ), closure_code );
        sprintf( outbuf, "ok\n" );
      } else {
        sprintf( outbuf, "could not create device channel error\n" );
        errors = -1;
      }
    }
  } else {
    sprintf( outbuf, "function takes three arguments (addr, port, callback) error!\n" );
    errors = -1;
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  if( errors ) {
    tcm_error( "%s: %s", __func__, outbuf );
    retval = sc -> F;
  } else {
    retval = mk_integer( sc, (long)p_server_sock_channel );
  }

  return(retval);
}


/*!
 *  return channel's connection state
 *
 *  \param sc pointer to scheme context
 *  \param args pointer to argument list, here one argument providing channel identifier
 *  \return true when channel open
 */
static pointer scm_is_channel_open( scheme *sc, pointer args )
{
  t_tcm_scheme* p_tcm_scheme = (t_tcm_scheme *)sc;
  pointer arg;
  pointer retval = sc->F;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  pointer closure_code;
  t_base_channel* p_base_channel;
  t_dev_channel* p_dev_channel;
  char* p_write_buf;

  while( args != sc->NIL )
  {
    if( i > 0 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes one arguments only error!\n" );
      errors = -1;
      break;
    }
    else if( i == 0  ) {
      if( is_integer( arg = pair_car(args)) ) {
        p_base_channel = (t_base_channel *) ivalue( arg );
        p_dev_channel = (t_dev_channel *) p_base_channel;
      } else {
        snprintf( outbuf, sizeof(outbuf), "first argument must be channel descriptor!\n" );
        errors = -1;
        break;
      }
    }

    args = pair_cdr( args );
    ++i;
  }

  if( ! errors ) {
    tcm_message( "%s: successfully executed\n", __func__ );
    if( p_base_channel && p_base_channel->is_open( p_base_channel ) ) {
      retval = sc->T;
    }
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  return(retval);
}

/*!
 * write bytes to channel
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument list, 1st argument is channel instance, 2nd argument is scheme integer or string value to be written
 * \return pointer to scheme integer value providing the number of bytes successfully writting
 */
static pointer scm_write_channel(scheme *sc, pointer args)
{
  t_tcm_scheme* p_tcm_scheme = (t_tcm_scheme *)sc;
  pointer arg;
  pointer retval;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  pointer closure_code;
  t_base_channel* p_base_channel;
  t_dev_channel* p_dev_channel;
  char* p_write_buf;
  int bytes_written = -1;

  while( args != sc->NIL )
  {
    if( i > 1 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes two arguments only error!\n" );
      errors = -1;
      break;
    }
    else if( i == 0  ) {
      if( is_integer( arg = pair_car(args)) ) {
        p_base_channel = (t_base_channel *) ivalue( arg );
        p_dev_channel = (t_dev_channel *) p_base_channel;
      } else {
        snprintf( outbuf, sizeof(outbuf), "first argument must be channel descriptor!\n" );
        errors = -1;
        break;
      }
    }
    else if( i == 1 ) {
      if( is_string( arg = pair_car(args) ) ) {
        p_write_buf = string_value( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "second argument must be string to write!\n" );
        errors = -1;
        break;
      }
    }

    args = pair_cdr( args );
    ++i;
  }

  if( ! errors ) {
    tcm_message( "%s: successfully executed\n", __func__ );
    bytes_written = p_base_channel->write( p_base_channel, p_write_buf, strlen( p_write_buf ) );
  } else {
    tcm_error( "%s: could not write to file %s, descriptor: %d error!\n", __func__, p_dev_channel->name, p_dev_channel->fd );
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  retval = mk_integer( sc, (long) bytes_written );
  return(retval);
}

/*!
 *  close communication channel instance
 *
 * \param sc pointer to scheme context
 * \param args pointer to argument list with one argument of channel instance to close
 * \return pointer to boolean value as result code, #t in case of success
 */
static pointer scm_close_channel(scheme *sc, pointer args)
{
  t_tcm_scheme* p_tcm_scheme = (t_tcm_scheme *)sc;
  pointer arg;
  pointer retval;
  int     i = 0;
  char    outbuf[80] = { '\0' };
  int     errors = 0;
  pointer closure_code;
  t_base_channel* p_base_channel;
  char* p_write_buf;
  int bytes_written = -1;

  while( args != sc->NIL )
  {
    if( i > 0 ) {
      snprintf( outbuf, sizeof(outbuf), "function takes one argument only error!\n" );
      errors = -1;
      break;
    }
    else if( i == 0  ) {
      if( is_integer( arg = pair_car(args)) ) {
        p_base_channel = (t_base_channel *) ivalue( arg );
      } else {
        snprintf( outbuf, sizeof(outbuf), "first argument must be channel descriptor!\n" );
        errors = -1;
        break;
      }
    }

    args = pair_cdr( args );
    ++i;
  }

  if( ! errors ) {
    /* clear symbol linkage to call back function to allow gc to release callback closure */
    scheme_define( sc, sc->global_env, mk_symbol( sc, p_base_channel->cb_symbol_name ), sc->NIL );
    p_base_channel->release( p_base_channel );
  }

  if( outbuf[0] != '\0' )
    putstr( sc, outbuf );

  if( errors ) {
    tcm_error( "%s: %s", __func__, outbuf );
    retval = sc -> F;
  } else {
    tcm_message( "%s: successfully executed\n", __func__ );
    retval = sc -> T;
  }

  return(retval);
}

/*!
 * returns the full qualified path name of the script installation directory
 *
 * usually /usr/bin or /usr/local/bin
 *
 * \param sc pointer to scheme instance data
 * \param args not used
 * \return pointer to scheme string with full qualified script installation path name
 */
static pointer scm_get_script_dir(scheme *sc, pointer args)
{
  const char* p_script_dir = SCHEMESCRIPTDIR;
  pointer retval;

  retval = mk_string( sc, p_script_dir );

  return( retval );
}


void init_tcm_ff( scheme* sc )
{
  scheme_define( sc, sc->global_env, mk_symbol( sc, "system" ), mk_foreign_func( sc, scm_system ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "sleep" ), mk_foreign_func( sc, scm_sleep ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "quit" ), mk_foreign_func( sc, scm_quit ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "make-dev-channel" ), mk_foreign_func( sc, scm_make_dev_channel ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "make-client-sock-channel" ), mk_foreign_func( sc, scm_make_client_sock_channel ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "make-server-sock-channel" ), mk_foreign_func( sc, scm_make_server_sock_channel ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "is-channel-open" ), mk_foreign_func( sc, scm_is_channel_open ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "write-channel" ), mk_foreign_func( sc, scm_write_channel ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "close-channel" ), mk_foreign_func( sc, scm_close_channel ) );
  scheme_define( sc, sc->global_env, mk_symbol( sc, "get-script-dir" ), mk_foreign_func( sc, scm_get_script_dir ) );
}

/*! @} */
