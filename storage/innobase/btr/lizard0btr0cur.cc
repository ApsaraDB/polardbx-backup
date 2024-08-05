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

/** @file btr/lizard0btr0cur.cc
 lizard b-tree.

 Created 2024-04-10 by Ting Yuan
 *******************************************************/

#include "lizard0btr0cur.h"
#include "btr0pcur.h"
#include "btr0sea.h"
#include "lizard0row.h"
#include "lizard0dict0mem.h"

namespace lizard {
/**
 * Attempts to position a persistent cursor on a clustered index record
 * based on the gpp_no read from the secondary index record.
 * @return        True if successful positioning, False otherwise
 */
bool btr_cur_guess_clust_by_gpp(dict_index_t *clust_idx,
                                const dict_index_t *sec_idx,
                                const dtuple_t *clust_ref, const rec_t *sec_rec,
                                btr_pcur_t *clust_pcur,
                                const ulint *sec_offsets, ulint latch_mode,
                                mtr_t *mtr) {
  ut_ad(sec_idx->n_s_gfields > 0);
  ut_ad(sec_offsets && sec_offsets[1] == sec_idx->n_fields);
  ut_ad(latch_mode == BTR_SEARCH_LEAF || latch_mode == BTR_MODIFY_LEAF);
  ut_ad(rec_offs_validate(sec_rec, sec_idx, sec_offsets));
  ut_ad(sec_idx->table == nullptr || !sec_idx->table->is_compressed());

  /** Phase 1: read gpp no from the sec rec. */
  bool clust_found = false;
  ulint up_match = 0;
  ulint low_match = 0;
  ulint idx_savepoint = 0;
  ulint block_savepoint = 0;
  buf_block_t *block = nullptr;
  ulint rw_latch = latch_mode;

  page_no_t gpp_no = lizard::row_get_gpp_no(sec_rec, sec_idx, sec_offsets);
  ut_ad(gpp_no != 0);

#ifdef UNIV_DEBUG
  DBUG_EXECUTE_IF("set_gpp_null", gpp_no = FIL_NULL;);
  DBUG_EXECUTE_IF("set_gpp_enum_page_in_space", gpp_no = dbug_gpp_no;);
#endif /* UNIV_DEBUG */

  /** Avoid acquiring the page latch recursively. */
  if (gpp_no == FIL_NULL || gpp_no == page_get_page_no(page_align(sec_rec)) ||
      clust_idx->table->is_compressed()) {
    goto func_exit;
  }

  /** Phase 2: fetch the page according to gpp_no. */
  idx_savepoint = mtr_set_savepoint(mtr);
  mtr_s_lock(dict_index_get_lock(clust_idx), mtr, UT_LOCATION_HERE);

  /** Fetch the page in Page_fetch::GPP_FETCH mode because the page may
   * have already been freed or out of tablespace. */
  block_savepoint = mtr_set_savepoint(mtr);
  block =
      buf_page_get_gen(page_id_t{dict_index_get_space(clust_idx), gpp_no},
                       dict_table_page_size(clust_idx->table), rw_latch,
                       nullptr, Page_fetch::GPP_FETCH, UT_LOCATION_HERE, mtr);

  mtr_release_s_latch_at_savepoint(mtr, idx_savepoint,
                                   dict_index_get_lock(clust_idx));

  if (block == nullptr ||
      !fil_page_index_page_check(buf_block_get_frame(block)) ||
      btr_page_get_index_id(buf_block_get_frame(block)) != clust_idx->id ||
      !page_is_leaf(buf_block_get_frame(block))) {
    goto func_exit;
  }

  ut_d(buf_page_mutex_enter(block));
  ut_ad(!block->page.file_page_was_freed);
  ut_d(buf_page_mutex_exit(block));

  /** Phase 3: search the clust rec in the target page. */
  clust_pcur->m_latch_mode = latch_mode;
  clust_pcur->m_search_mode = PAGE_CUR_LE;
  clust_pcur->m_old_stored = false;

  page_cur_search_with_match(block, clust_idx, clust_ref,
                             clust_pcur->m_search_mode, &up_match, &low_match,
                             clust_pcur->get_page_cur(), nullptr);

  clust_pcur->m_pos_state = BTR_PCUR_IS_POSITIONED;
  clust_pcur->m_btr_cur.index = clust_idx;
  clust_pcur->m_btr_cur.flag = BTR_CUR_BINARY;

  /** Phase 4: rec validation. */
  if (!page_rec_is_user_rec(clust_pcur->get_rec()) ||
      low_match < dict_index_get_n_unique(clust_idx)) {
    clust_pcur->reset_btr_cur();
    goto func_exit;
  }

  clust_found = true;

func_exit:
  if (!clust_found) {
    /** Release the page if the found rec is mismatched. */
    if (block) {
      mtr_release_block_at_savepoint(mtr, block_savepoint, block);
    }
  }

  sec_idx->gpp_stat(clust_found);

  return clust_found;
}

}  // namespace lizard
