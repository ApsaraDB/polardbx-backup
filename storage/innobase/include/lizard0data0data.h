/*****************************************************************************

Copyright (c) 2013, 2024, Alibaba and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/lizard0data0data.h
   Lizard SQL data field and tuple
  

 Created 2024-03-26 by Jianwei.zhao
 *******************************************************/
#ifndef lizard0data0data_h
#define lizard0data0data_h

#include "data0types.h"
#include "univ.i"

struct dict_index_t;

namespace lizard {

/** Dtuple always has GPP_NO virtual field.
 *  @param[in]		tuple
 *
 *  @retval		virtual GPP_NO field.
 * */
extern dfield_t *dtuple_get_v_gfield(const dtuple_t *tuple);

/** Copy GPP dfield from row to index. */
extern void dtuple_copy_v_gfield(dtuple_t *entry, const dtuple_t *row);

/** Set Index GPP dfield from row . */
extern void dtuple_set_data_v_gfield(dtuple_t *entry, const dtuple_t *row);

/**
  Get ordered field number from dtuple for ibuf.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
extern ulint ibuf_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                              const dict_index_t *index);

/**
  Get ordered field number from dtuple for row search.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
extern ulint row_search_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                                    const dict_index_t *index);

/**
  Get ordered field number from dtuple for row upd.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
extern ulint row_upd_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                                 const dict_index_t *index);

/**
  Get ordered field number from dtuple for row ver.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
extern ulint row_ver_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                                 const dict_index_t *index);

/** Compare two data tuples of secondary index.
  @param[in] tuple1    first data tuple
  @param[in] tuple2    second data tuple
  @param[in] sec_index secondary index
  @return whether tuple1==tuple2 */
extern bool row_ver_sec_dtuple_coll_eq(const dtuple_t *tuple1,
                                       const dtuple_t *tuple2,
                                       const dict_index_t *sec_index);

}  // namespace lizard
#endif

