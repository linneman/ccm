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
#include <tcm_scheme_ext.h>
#include <tcm_config.h>
#include <tcm_log.h>
#include <fmemopen.h>

#define PORT       "37147" /* Port to listen on */
#define BACKLOG        10  /* Passed to listen() */
#define MAX_BUF_SIZE 1000

#define INIT_FILE1  "./init.scm"
#define INIT_FILE2  "/etc/init.scm"
#define INIT_FILE3  "/usr/local/etc/init.scm"

#define TCM_INIT_FILE1  SCHEMESCRIPTDIR "/tcm.scm"
#define TCM_INIT_FILE2  "/etc/tcm.scm"
#define TCM_INIT_FILE3  "/usr/local/etc/tcm.scm"

/**
 * illustration of a C function binding

  try:
  (func1 "hello" 42 1.24)
 */
static pointer func1(scheme *sc, pointer args)
{
  pointer arg;
  pointer retval;
  char    *strarg;
  double  realnumber;
  int     intnumber;
  int     i = 1;
  char    outbuf[80];

  while( args != sc->NIL )
  {
    outbuf[0] = '\0';
    if( is_string( arg = pair_car(args)) ) {
      strarg = string_value( arg );
      snprintf( outbuf, sizeof(outbuf), "argument %d is string %s\n", i++, strarg );
    }
    else if( is_real( arg = pair_car(args) ) ) {
      realnumber = rvalue( arg );
      snprintf( outbuf, sizeof(outbuf), "argument %d is real number %lf\n", i++, realnumber );
    }
    else if( is_integer( arg = pair_car(args) ) ) {
      intnumber = ivalue( arg );
      snprintf( outbuf, sizeof(outbuf), "argument %d is integer number %d\n", i++, intnumber );
    }

    if( outbuf[0] != '\0' )
      putstr( sc, outbuf );

    args = pair_cdr( args );
  }

  if( i > 1 )
    retval = sc -> T;
  else
    retval = sc -> F;

  return(retval);
}


static int read_init_file( scheme* sc, const char* filename )
{
  FILE* fp;

  fp = fopen( filename, "r" );
  if( fp != NULL )
  {
    tcm_message( "initialized scheme with init file %s\n", filename );

    scheme_load_named_file( sc, fp, filename );
    if( sc->retcode!=0 ) {
      tcm_message( "Errors encountered reading %s\n", filename );
    }
    fclose( fp );
    return sc->retcode;
  }
  else
    return -1;
}


static void init_ff( scheme* sc )
{
  /* illustration how to define a "foreign" function
     implemented in C */
  scheme_define( sc, sc->global_env, mk_symbol( sc, "func1" ), mk_foreign_func( sc, func1 ) );
}


void tcm_release_scheme( t_tcm_scheme* p )
{
  if( p ) {
    if( p->p_repl_server )
      icom_kill_server_handlers( p->p_repl_server );

    scheme_deinit( & p->sc );
    pthread_mutex_destroy( &p->mutex );
    cul_free( p );
  }
}


static int icom_evt_socket( t_icom_evt* p_evt )
{
  return ((struct s_sock_server *)(p_evt->p_source))->fd;
}


static int repl_evt_cb( t_icom_evt* p_evt )
{
  t_tcm_scheme* p = (t_tcm_scheme *) p_evt->p_user_ctx;
  FILE *fdin, *fdout;
  int socket;
  int errors = 0;
  char greet[128];

  sprintf( greet, "tinyscheme %s\nts> ",
           tiny_scheme_version );

  if( p_evt->type == ICOM_EVT_SERVER_CON ) {
    tcm_message( "tcm repl is connected!\n" );
    errors = icom_reply_to_address( p_evt, greet, strlen( greet ) );
  } else if( p_evt->type == ICOM_EVT_SERVER_DIS ) {
    tcm_message( "tcm repl is disconnected\n" );
  }
  else if( p_evt->type == ICOM_EVT_SERVER_DATA ) {
    p_evt->p_data[p_evt->max_data_size - 1] = '\0'; /* enfore null termination */
    tcm_message( "received repl request: %s\n", p_evt->p_data );

    pthread_mutex_lock( & p->mutex );
    /* set standard input and output ports */
    socket = icom_evt_socket( p_evt );
    p->sc.interactive_repl=1;
    fdin = fmemopen( p_evt->p_data, strlen(p_evt->p_data), "r" );
    fdout = fdopen( dup(socket), "w" );
    scheme_set_input_port_file( &p->sc, fdin );
    scheme_set_output_port_file( &p->sc, fdout );
    scheme_load_named_file( &p->sc, fdin, 0);
    errors = p->sc.retcode;
    fflush( fdout );
    p->sc.interactive_repl=0;
    fclose( fdout );
    fclose( fdin );
    pthread_mutex_unlock( & p->mutex );
  }

  if( errors ) {
    tcm_error( "%s: scheme evaluation error occured\n", __func__ );
  }

  return errors;
}


int tcm_load_scheme_file( t_tcm_scheme* p, const char* filename )
{
  FILE* fp;
  int errors = 0;

  fp = fopen( filename, "r" );

  if( fp != NULL ) {
    pthread_mutex_lock( & p->mutex );

    scheme_load_named_file( & p->sc, fp, filename );
    if( p->sc.retcode!=0 ) {
      tcm_message( "%s: errors encountered evaluating %s\n", __func__, filename );
    }
    errors =  p->sc.retcode;

    pthread_mutex_unlock( & p->mutex );

    fclose( fp );
  }
  else {
    tcm_message( "%s: could not load file %s error\n", __func__, filename );
    errors = -1;
  }

  return errors;
}


int tcm_load_scheme_desc( t_tcm_scheme* p, FILE *fdin, FILE *fdout )
{
  int errors = 0;

  tcm_message( "%s ... \n", __func__ );

  pthread_mutex_lock( & p->mutex );
  p->sc.interactive_repl=1;
  scheme_set_input_port_file( &p->sc, fdin );
  scheme_set_output_port_file( &p->sc, fdout );
  scheme_load_named_file( &p->sc, fdin, 0);
  errors = p->sc.retcode;
  fflush( fdout );
  p->sc.interactive_repl=0;
  pthread_mutex_unlock( & p->mutex );

  return errors;
}


int tcm_load_scheme_string( t_tcm_scheme* p, char* string, t_tcm_scheme_ret_val* p_ret )
{
  int errors = 0;
  long scheme_errors;
  FILE *fdin, *fdout;

  pthread_mutex_lock( & p->mutex );

  p->sc.interactive_repl=0;
  fdin = fmemopen( string, strlen(string), "r" );
  fdout = fopen( "/dev/null", "w" );
  scheme_set_input_port_file( &p->sc, fdin );
  scheme_set_output_port_file( &p->sc, fdout );
  scheme_load_named_file( &p->sc, fdin, 0);
  errors = p->sc.retcode;
  fflush( fdout );
  p->sc.interactive_repl=0;
  fclose( fdout );
  fclose( fdin );

  if( p->sc.value == p->sc.T )
  {
    if( p_ret ) {
      p_ret->v.bval = 1;
      p_ret->t = t_tcm_scheme_bool;
    }
    scheme_errors = 0L;
  }/* scheme functions returns logical true */
  if( p->sc.value == p->sc.F )
  {
    if( p_ret ) {
      p_ret->v.bval = 0;
      p_ret->t = t_tcm_scheme_bool;
    }
    scheme_errors = -1L;
  }/* scheme functions returns logical true */
  else if( is_integer( p->sc.value ) )
  {
    if( p_ret ) {
      p_ret->v.ival = ivalue( p->sc.value );
      p_ret->t = t_tcm_scheme_integer;
    }
    scheme_errors = ivalue( p->sc.value );
  } /* scheme function returns integer */
  else if( is_real( p->sc.value ) )
  {
    if( p_ret ) {
      p_ret->v.rval = rvalue( p->sc.value );
      p_ret->t = t_tcm_scheme_real;
    }
    scheme_errors = ivalue( p->sc.value );
  } /* scheme function returns integer */
  else if( is_string( p->sc.value ) )
  {
    if( p_ret ) {
      strncpy( p_ret->v.str, string_value( p->sc.value ), sizeof( p_ret->v.str ) );
      p_ret->t = t_tcm_scheme_string;
    }
  } /* scheme function returns integer */
  pthread_mutex_unlock( & p->mutex );

  tcm_message("%s: -> return code: %d, return value: %ld\n", __func__, errors, scheme_errors );

  if( !errors && scheme_errors < 0L /* treat #f and negative values as errors */ )
    errors = -2;

  return errors;
}


t_tcm_scheme* tcm_init_scheme( t_tcm_server_ctx* p_tcm_server_ctx )
{
  t_tcm_scheme* p;
  t_icom_server_decl decl_table[1];
  const int decl_table_len = sizeof(decl_table) / sizeof( t_icom_server_decl );

  decl_table[0].addr.sock_family = AF_INET;
  strncpy( decl_table[0].addr.address, g_tcm_scheme_ip_address, sizeof(decl_table[0].addr.address) );
  decl_table[0].addr.port = g_tcm_scheme_ip_port;
  decl_table[0].max_connections = 10;

  p = cul_malloc( sizeof( t_tcm_scheme ) );

  if( p == NULL ) {
    tcm_error( "%s: out of memory error!\n", __func__ );
    return NULL;
  }

  memset( p, sizeof( t_tcm_scheme ), 0 );
  p->p_tcm_server_ctx = p_tcm_server_ctx;

  if( pthread_mutex_init( &p->mutex, 0 ) != 0 ) {
    tcm_error( "%s: could initialize scheme interpreter error\n", __func__ );
    cul_free( p );
    return NULL;
  }

  /* intialize the scheme object */
  if( !scheme_init(&p->sc) ) {
    tcm_error( "%s: could not initialize scheme interpreter!\n", __func__ );
    pthread_mutex_destroy( &p->mutex );
    cul_free( p );
    return NULL;
  }

  /* initialize tcm specific add on functions */
  init_ff( &p->sc );
  init_tcm_ff( &p->sc );

  /* read scheme initialization file, try various locations */
  if( read_init_file( &p->sc, INIT_FILE1 ) )
    if( read_init_file( &p->sc, INIT_FILE2 ) )
      if( read_init_file( &p->sc, INIT_FILE3 ) )
        tcm_message( "%s: no init file read!\n", __func__ );

  /* read tcm scheme function initialization file */
  if( read_init_file( &p->sc, TCM_INIT_FILE1 ) )
    if( read_init_file( &p->sc, TCM_INIT_FILE2 ) )
      if( read_init_file( &p->sc, TCM_INIT_FILE3 ) )
        tcm_message( "%s: no tcm init file read!\n", __func__ );

  if( g_tcm_scheme_ip_port ) {
    p->p_repl_server = icom_create_server_handlers( decl_table, decl_table_len, 4096, 10, repl_evt_cb, p );
    if( p->p_repl_server == NULL ) {
      tcm_error( "%s: could not create repl server error!\n", __func__ );
    } else {
      tcm_message( "%s: repl listenting to address %d\n", __func__, g_tcm_scheme_ip_port );
    }
  }

  return p;
}
