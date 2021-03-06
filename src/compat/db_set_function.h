/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

//
// db_set_function - db_set functions
//

#ifndef _DB_SET_FUNCTION_
#define _DB_SET_FUNCTION_

#include "dbtype_def.h"

#ifdef __cplusplus
extern "C"
{
#endif				// C++

  extern int db_set_compare (const DB_VALUE * value1, const DB_VALUE * value2);

  extern DB_COLLECTION *db_set_create (DB_OBJECT * classobj, const char *name);
  extern DB_COLLECTION *db_set_create_basic (DB_OBJECT * classobj, const char *name);
  extern DB_COLLECTION *db_set_create_multi (DB_OBJECT * classobj, const char *name);
  extern DB_COLLECTION *db_seq_create (DB_OBJECT * classobj, const char *name, int size);
  extern int db_set_free (DB_COLLECTION * set);
  extern int db_set_filter (DB_COLLECTION * set);
  extern int db_set_add (DB_COLLECTION * set, DB_VALUE * value);
  extern int db_set_get (DB_COLLECTION * set, int element_index, DB_VALUE * value);
  extern int db_set_drop (DB_COLLECTION * set, DB_VALUE * value);
  extern int db_set_size (DB_COLLECTION * set);
  extern int db_set_cardinality (DB_COLLECTION * set);
  extern int db_set_ismember (DB_COLLECTION * set, DB_VALUE * value);
  extern int db_set_isempty (DB_COLLECTION * set);
  extern int db_set_has_null (DB_COLLECTION * set);
  extern int db_set_print (DB_COLLECTION * set);
  extern DB_TYPE db_set_type (DB_COLLECTION * set);
  extern DB_COLLECTION *db_set_copy (DB_COLLECTION * set);
  extern int db_seq_get (DB_COLLECTION * set, int element_index, DB_VALUE * value);
  extern int db_seq_put (DB_COLLECTION * set, int element_index, DB_VALUE * value);
  extern int db_seq_insert (DB_COLLECTION * set, int element_index, DB_VALUE * value);
  extern int db_seq_drop (DB_COLLECTION * set, int element_index);
  extern int db_seq_size (DB_COLLECTION * set);
  extern int db_seq_cardinality (DB_COLLECTION * set);
  extern int db_seq_print (DB_COLLECTION * set);
  extern int db_seq_find (DB_COLLECTION * set, DB_VALUE * value, int element_index);
  extern int db_seq_free (DB_SEQ * seq);
  extern int db_seq_filter (DB_SEQ * seq);
  extern DB_SEQ *db_seq_copy (DB_SEQ * seq);

  /* Collection functions */
  extern DB_COLLECTION *db_col_create (DB_TYPE type, int size, DB_DOMAIN * domain);
  extern DB_COLLECTION *db_col_copy (DB_COLLECTION * col);
  extern int db_col_filter (DB_COLLECTION * col);
  extern int db_col_free (DB_COLLECTION * col);
  extern int db_col_coerce (DB_COLLECTION * col, DB_DOMAIN * domain);

  extern int db_col_size (DB_COLLECTION * col);
  extern int db_col_cardinality (DB_COLLECTION * col);
  extern DB_TYPE db_col_type (DB_COLLECTION * col);
  extern DB_DOMAIN *db_col_domain (DB_COLLECTION * col);
  extern int db_col_ismember (DB_COLLECTION * col, DB_VALUE * value);
  extern int db_col_find (DB_COLLECTION * col, DB_VALUE * value, int starting_index, int *found_index);
  extern int db_col_add (DB_COLLECTION * col, DB_VALUE * value);
  extern int db_col_drop (DB_COLLECTION * col, DB_VALUE * value, int all);
  extern int db_col_drop_element (DB_COLLECTION * col, int element_index);

  extern int db_col_drop_nulls (DB_COLLECTION * col);

  extern int db_col_get (DB_COLLECTION * col, int element_index, DB_VALUE * value);
  extern int db_col_put (DB_COLLECTION * col, int element_index, DB_VALUE * value);
  extern int db_col_insert (DB_COLLECTION * col, int element_index, DB_VALUE * value);

  extern int db_col_print (DB_COLLECTION * col);
  extern int db_col_fprint (FILE * fp, DB_COLLECTION * col);

#ifdef __cplusplus
}				// extern "C"
#endif				// C++

#endif				// !_DB_SET_FUNCTION_
