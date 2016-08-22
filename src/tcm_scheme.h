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

#ifndef TCM_SCHEME_H
#define TCM_SCHEME_H

#include <intercom/server.h>
#include <tinyscheme/scheme.h>
#include <tinyscheme/dynload.h>
#include <tcm_server.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
    \file tcm_scheme.h
    \brief scheme script interpreter
    \addtogroup scheme
    @{
 */


/*! scheme interpreter state data */
typedef struct s_tcm_scheme {
  scheme                      sc;                       /*!< scheme interpreter state */
  pthread_mutex_t             mutex;                    /*!< access protection to avoid scheme rc */
  t_icom_server_state*        p_repl_server;            /*!< repl server */
  t_tcm_server_ctx*           p_tcm_server_ctx;         /*!< back reference to server ctx */
} t_tcm_scheme;


/*!
 * create scheme interpreter instance
 *
 * This  function  creates   an  instance  for  interpreting   scheme  code  and
 * instanziates a scheme read-eval-print-loop  (repl) in case corresponding port
 * is specified within the global configuration.
 *
 * \param p_tcm_server_ctx pointer to main instance object
 * \return pointer to the newly instantiated object or NULL in case of error
 */
t_tcm_scheme* tcm_init_scheme( t_tcm_server_ctx* p_tcm_server_ctx );


/*!
 * release scheme interpreter instance
 *
 * stops repl and releases all scheme instance data
 *
 * \param p pointer to the scheme object to be released
 */
void tcm_release_scheme( t_tcm_scheme* p );


/*!
 * load and evaluate scheme file
 *
 * \param p pointer to the scheme object
 * \param filename full qualified filename where to read scheme data from
 * \return 0 in case of success, otherwise negative error code
 */
int tcm_load_scheme_file( t_tcm_scheme* p, const char* filename );


/*!
 * load and evaluate scheme data from buffered file descriptors
 *
 * \param p pointer to the scheme object
 * \param fdin input file descriptor where data is retrieved from
 * \param fdout output file descriptor where output data is written to
 * \return 0 in case of success, otherwise negative error code
 */
int tcm_load_scheme_desc( t_tcm_scheme* p, FILE *fdin, FILE *fdout );



/*! union type of scheme scalar return value */
typedef enum {
  t_tcm_scheme_string,                                  /*!< scheme string */
  t_tcm_scheme_real,                                    /*!< scheme real number */
  t_tcm_scheme_integer,                                 /*!< scheme integer value */
  t_tcm_scheme_bool                                     /*!< scheme boolean value */
} t_tcm_scheme_ret_val_type;


/*! scheme scalar return value */
typedef struct {
  union {
    char str[256];                                      /*!< string value */
    double rval;                                        /*!< real value */
    long ival;                                          /*!< integer value */
    int bval;                                           /*!< boolean value */
  } v;                                                  /*!< union holding scheme return variable */
  t_tcm_scheme_ret_val_type t;                          /*!< scheme return value type */
} t_tcm_scheme_ret_val;


/*!
 * evaluate scheme data from string buffer
 *
 * \param p pointer to the scheme object
 * \param string zero terminated input data to evaluate
 * \param p_ret optional pointer to write back scalar return value
 * \return 0 in case of success, otherwise negative error code
 */
int tcm_load_scheme_string( t_tcm_scheme* p, char* string, t_tcm_scheme_ret_val* p_ret );


/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* #ifndef TCM_SCHEME_H */
