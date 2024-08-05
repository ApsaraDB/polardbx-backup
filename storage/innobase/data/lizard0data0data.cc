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

/** @file data/lizard0data0data.cc
   Lizard SQL data field and tuple
  

 Created 2024-03-26 by Jianwei.zhao
 *******************************************************/

#include "lizard0data0data.h"
#include "dict0mem.h"
#include "lizard0dict.h"
#include "rem0cmp.h"

/**
 * Read GPP_NO from dfield data which only exist in memory.
 * */
gpp_no_t dtuple_t::read_v_gpp_no() const {
  dfield_t *field = v_gfield;
  ut_ad(field);
  ut_ad(field->type.mtype == DATA_SYS_GPP &&
        field->type.prtype == DATA_NOT_NULL);

  return mach_read_from_4((byte *)field->data);
}

/**
 * Read GPP_NO from dfield data which is stored in sec index.
 * */
gpp_no_t dtuple_t::read_s_gpp_no() const {
  const dfield_t &field = fields[n_fields - 1];
  ut_ad(field.type.mtype == DATA_SYS_GPP && field.type.prtype == DATA_NOT_NULL);
  gpp_no_t s_gpp_no = mach_read_from_4((byte *)field.data);
  return s_gpp_no;
}

namespace lizard {

/** Dtuple always has GPP_NO virtual field.
 *  @param[in]		tuple
 *
 *  @retval		virtual GPP_NO field.
 * */
dfield_t *dtuple_get_v_gfield(const dtuple_t *tuple) {
  ut_ad(tuple->v_gfield);
  return tuple->v_gfield;
}

/** Copy GPP dfield from row to index. */
void dtuple_copy_v_gfield(dtuple_t *entry, const dtuple_t *row) {
  dfield_t *dfield;
  dfield_t *dfield2;
  ut_ad(entry && row);

  dfield = lizard::dtuple_get_v_gfield(entry);
  dfield2 = lizard::dtuple_get_v_gfield(row);
  dfield_copy(dfield, dfield2);
}

/** Set Index GPP dfield from row . */
void dtuple_set_data_v_gfield(dtuple_t *entry, const dtuple_t *row) {
  dfield_t *dfield;
  dfield_t *dfield2;
  ulint len;
  ut_ad(entry && row);

  dfield = lizard::dtuple_get_v_gfield(entry);
  dfield2 = lizard::dtuple_get_v_gfield(row);

  len = dfield2->len;
  ut_ad(len == DATA_GPP_NO_LEN);
  dfield_set_data(dfield, dfield2->data, len);
}

/**
  Get ordered field number from dtuple for ibuf.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
ulint ibuf_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                       const dict_index_t *index) {
  ut_ad(entry && index && !index->is_clustered());
  assert_lizard_dict_index_gstored_check(index);
  ut_ad(dtuple_get_n_fields(entry) == index->n_fields);
  return dtuple_get_n_fields(entry) - index->n_s_gfields;
}

/**
  Get ordered field number from dtuple for row search.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
ulint row_search_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                             const dict_index_t *index) {
  ut_ad(entry && index && !index->is_clustered());
  assert_lizard_dict_index_gstored_check(index);
  ut_ad(dtuple_get_n_fields(entry) == index->n_fields);
  return dtuple_get_n_fields(entry) - index->n_s_gfields;
}

/**
  Get ordered field number from dtuple for row upd.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
ulint row_upd_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                          const dict_index_t *index) {
  ut_ad(entry && index && !index->is_clustered());
  assert_lizard_dict_index_gstored_check(index);
  ut_ad(dtuple_get_n_fields(entry) == index->n_fields);
  return dtuple_get_n_fields(entry) - index->n_s_gfields;
}

/**
  Get ordered field number from dtuple for row ver.
  @param[in]      entry     dtuple_t
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
ulint row_ver_dtuple_get_ordered_n_fields(const dtuple_t *entry,
                                          const dict_index_t *index) {
  ut_ad(entry && index && !index->is_clustered());
  assert_lizard_dict_index_gstored_check(index);
  ut_ad(dtuple_get_n_fields(entry) == index->n_fields);
  return dtuple_get_n_fields(entry) - index->n_s_gfields;
}

/** Compare two data tuples of secondary index.
  @param[in] tuple1    first data tuple
  @param[in] tuple2    second data tuple
  @param[in] sec_index secondary index
  @return whether tuple1==tuple2 */
bool row_ver_sec_dtuple_coll_eq(const dtuple_t *tuple1, const dtuple_t *tuple2,
                                const dict_index_t *sec_index) {
  ulint n_fields;
  ulint i;
  int cmp;

  ut_ad(tuple1 != nullptr);
  ut_ad(tuple2 != nullptr);
  ut_ad(tuple1->magic_n == dtuple_t::MAGIC_N);
  ut_ad(tuple2->magic_n == dtuple_t::MAGIC_N);
  ut_ad(dtuple_check_typed(tuple1));
  ut_ad(dtuple_check_typed(tuple2));
  ut_ad(!sec_index->is_clustered());
  assert_lizard_dict_index_gstored_check(sec_index);

  n_fields = row_ver_dtuple_get_ordered_n_fields(tuple1, sec_index);

  cmp = (int)n_fields -
        (int)row_ver_dtuple_get_ordered_n_fields(tuple2, sec_index);

  for (i = 0; cmp == 0 && i < n_fields; i++) {
    dfield_t *field1 = dtuple_get_nth_field(tuple1, i);
    const dfield_t *field2 = dtuple_get_nth_field(tuple2, i);

    ut_ad(dfield_get_len(field2) != UNIV_NO_INDEX_VALUE);
    ut_ad(!dfield_is_multi_value(field2) ||
          dfield_get_len(field2) != UNIV_MULTI_VALUE_ARRAY_MARKER);

    if (dfield_is_multi_value(field1)) {
      cmp = cmp_multi_value_dfield_dfield(field1, field2);
    } else {
      /* Equality comparison does not care about ASC/DESC. */
      cmp = cmp_dfield_dfield(field1, field2, true);
    }
  }

  return (cmp == 0);
}

}  // namespace lizard
