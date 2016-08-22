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
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <tcm_config.h>
#include <tcm_log.h>
#include <olcutils/alloc.h>
#include <olcutils/cfg_string.h>


char g_tcm_scheme_ip_address[TCM_MAX_ADDR_LEN] = { "0.0.0.0" };
int  g_tcm_scheme_ip_port = 37147;


static void* free_string_val( void* p )
{
  string_release( (string_t*)p );
  return NULL;
}

static int string2int( const string_t* s, int* p_i, const int minval, const int maxval )
{
  char tmp_cstring[30];
  char* end = 0;
  long l;
  int retcode = 0;

  string_tmp_cstring_from( s, tmp_cstring, sizeof( tmp_cstring ) );
  l = strtol( tmp_cstring, &end, 10 );
  if( l == LONG_MIN || l == LONG_MAX ) {
    retcode = -1;
  } else  {
    if( l >= minval && l <=maxval ) {
      *p_i = (int)l;
    } else {
      retcode = -2;
    }
  }

  return retcode;
}

int tcm_init_config(void)
{
  struct passwd* pw = getpwuid(getuid());
  char local_conf_path[TCM_MAX_PATH];
  char global_conf_path1[TCM_MAX_PATH], global_conf_path2[TCM_MAX_PATH];
  char* pUsedConfFileName;
  char* p_conf_data;
  int conf_data_len;
  FILE* fp;

  int retcode = 0;

  p_conf_data = (char *) cul_malloc( TCM_MAX_CONF_SIZE );
  if( ! p_conf_data ) {
    tcm_error("%s: out of memory error!\n", __func__ );
    return -1;
  }

  memset( p_conf_data, 0, TCM_MAX_CONF_SIZE );


  /* intialize path to local and global conf files */
  snprintf( local_conf_path, sizeof(local_conf_path), "%s/.tcm.rc", pw->pw_dir );
  snprintf( global_conf_path1, sizeof(global_conf_path1), "/etc/tcm.rc" );
  snprintf( global_conf_path2, sizeof(global_conf_path2), "/usr/local/etc/tcm.rc" );

  fp = fopen(local_conf_path, "r" );
  pUsedConfFileName = local_conf_path;
  if( fp ==  NULL )
  {
    fp = fopen(global_conf_path1, "r" );
    pUsedConfFileName = global_conf_path1;

    if( fp == NULL )
    {
      fp = fopen(global_conf_path2, "r" );
      pUsedConfFileName = global_conf_path2;
    }
  }

  if( fp == NULL )
  {
    tcm_error( "%s: configuration data neither at %s nor at %s or %s given!\n",
               __func__, local_conf_path, global_conf_path1, global_conf_path2 );
    cul_free( p_conf_data );
    return -1;
  }

  conf_data_len = fread( p_conf_data, 1, TCM_MAX_CONF_SIZE-1 /*-1 for null term. ensurance! */, fp );
  if( conf_data_len > 0 ) {
    string_t* s = string_new_from( p_conf_data );
    hm_t* params;
    hm_leaf_node_t* ln;

    params = cfgstring_parse( s );

    ln = hm_find( params, cstring_hash( "scheme-server-ip-address" ) );
    if( ln ) {
      string_tmp_cstring_from( ln->val, g_tcm_scheme_ip_address, sizeof( g_tcm_scheme_ip_address ) );
      tcm_message("%s: overwrite default IP address with %s\n", __func__, g_tcm_scheme_ip_address );
    }

    ln = hm_find( params, cstring_hash( "scheme-server-ip-port" ) );
    if( ln ) {
      if( ! string2int( ln->val, & g_tcm_scheme_ip_port, 0, 65535 ) ) {
        tcm_message("%s: overwrite default IP port with %d\n", __func__, g_tcm_scheme_ip_port );
      } else {
        tcm_error("%s: could not parse IP port argument error!\n", __func__ );
      }
    }

    string_release( s );
    hm_free_deep( params, 0, free_string_val );

  } else {
    tcm_error( "%s: could not read configuration data error!\n" );
  }

  cul_free( p_conf_data );
  fclose( fp );
  return retcode;
}
