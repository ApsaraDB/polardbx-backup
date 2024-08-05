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

/** @file include/lizard0dict0mem.h
 Special dictionary memory object operation.

 Created 2024-03-26 by Jianwei.zhao
 *******************************************************/

#ifndef lizard0dict0mem_h
#define lizard0dict0mem_h

#include "dict0mem.h"

#include "lizard0data0types.h"

namespace lizard {

/** Whether to enable clustered index record inference during the scan. */
extern bool index_scan_guess_clust_enabled;

/** Whether to enable clustered index record inference during the purge. */
extern bool index_purge_guess_clust_enabled;

/** Whether to enable clustered index record inference during the locking. */
extern bool index_lock_guess_clust_enabled;

#ifdef UNIV_DEBUG
extern gpp_no_t dbug_gpp_no;
#endif /* UNIV_DEBUG */

/** Build column definition for GPP_NO.
 *
 * @param[in]	table
 * @param[in]	heap
 *
 * @retval	GPP_NO column
 * */
extern dict_col_t *dict_mem_table_add_v_gcol(dict_table_t *table,
                                             mem_heap_t *heap);


}  // namespace lizard
#endif
