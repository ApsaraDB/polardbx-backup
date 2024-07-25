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

/** @file dict/lizard0dict0mem.cc
 Special dictionary memory object operation.

 Created 2024-03-26 by Jianwei.zhao
 *******************************************************/
#include "dict0mem.h"

#include "lizard0dict.h"
#include "lizard0dict0mem.h"

namespace lizard {

/** Whether to enable clustered index record inference during the scan. */
bool index_scan_guess_clust_enabled = true;

/** Whether to enable clustered index record inference during the purge. */
bool index_purge_guess_clust_enabled = true;

/** Whether to enable clustered index record inference during the locking. */
bool index_lock_guess_clust_enabled = true;

#ifdef UNIV_DEBUG
gpp_no_t dbug_gpp_no = PAGE_NO_MAX;
#endif /* UNIV_DEBUG */

/** Build column definition for GPP_NO.
 *
 * @param[in]	table
 * @param[in]	heap
 *
 * @retval	GPP_NO column
 * */
dict_col_t *dict_mem_table_add_v_gcol(dict_table_t *table, mem_heap_t *heap) {
  dict_col_t *col = nullptr;
  ut_ad(table);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  /** Predefined physical position, It's not necessary to reposition it from
   * dd_column since dd didn't save GPP_NO column definition. */
  const uint32_t phy_pos = DATA_GPP_NO;
  const uint8_t v_added = 0;
  const uint8_t v_dropped = 0;

  col = dict_table_get_v_gcol(table);

  dict_mem_fill_column_struct(col, DATA_GPP_NO, DATA_SYS_GPP, DATA_NOT_NULL,
                              DATA_GPP_NO_LEN, false, phy_pos, v_added,
                              v_dropped);

  return col;
}

}  // namespace lizard
