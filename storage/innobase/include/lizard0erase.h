/*****************************************************************************

Copyright (c) 1996, 2024, Oracle and/or its affiliates.

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

/** @file include/lizard0erase.h
 Erase old versions for semi-purge

 Created 13/05/2024 jiyang.zhang
 *******************************************************/

#ifndef lizard0erase_h
#define lizard0erase_h

#include "trx0purge.h"

class THD;

typedef purge_iter_t erase_iter_t;

namespace lizard {

struct trx_erase_t {
  /** The query graph which will do the parallelized erase operation */
  que_t *query;

  /** Count of total tasks submitted to the task queue */
  volatile ulint n_submitted;

  /** Count of total tasks completed */
  std::atomic<ulint> n_completed;

  /* The following two fields form the 'erase pointer' which advances
  during a erase, and which is used in history list truncation */

  /** Limit up to which we have read and parsed the UNDO log records.  Not
  necessarily erased from the indexes.  Note that this can never be less than
  the limit below, we check for this invariant in lizard0erase.cc */
  erase_iter_t iter;

  /** The 'erase pointer' which advances during a erase, and which is used in
  history list truncation */
  erase_iter_t limit;
#ifdef UNIV_DEBUG
  /** Indicate 'erase pointer' which have erased already accurately. */
  erase_iter_t done;
#endif /* UNIV_DEBUG */

  /** true if the info of the next record to erase is stored below: if yes, then
  the commit number and the undo number of the record are stored in
  scn and undo_no above */
  bool next_stored;

  /** Rollback segment for the next undo record to erase */
  trx_rseg_t *rseg;

  /** Page number for the next undo record to erase, page number of the log
  header, if dummy record */
  page_no_t page_no;

  /** Page offset for the next undo record to erase, 0 if the dummy record */
  ulint offset;

  /** Header page of the undo log where the next record to erase belongs */
  page_no_t hdr_page_no;

  /** Header byte offset on the page */
  ulint hdr_offset;

  /** TXN address of the undo log */
  slot_addr_t txn_addr;

  /** Heap for reading the undo log records */
  mem_heap_t *heap;

  /** All transactions whose scn <= erased_scn must have been erased.
  Only the erase sys coordinator thread and recover thread can modify it. */
  std::atomic<scn_t> erased_scn;

  /** Similar with erased_scn */
  lizard::Erased_gcn erased_gcn;

  void push_erased(const commit_order_t &ommt);
};

extern trx_erase_t *erase_sys;

/** Initialize in-memory erase structures */
extern void trx_erase_sys_mem_create();

/** Creates the global erase system control structure and inits the history
mutex.
@param[in]      n_purge_threads   number of purge threads */
extern void trx_erase_sys_initialize(uint32_t n_purge_threads);

extern void trx_erase_sys_close();
/**
  Get last (oldest) log header from semi purge list.
  @params[in]   rseg            update undo rollback segment
  @params[out]  log header address

  @retval	commit mark of log header
*/
extern commit_mark_t trx_erase_get_last_log(trx_rseg_t *rseg, fil_addr_t &addr,
                                            rseg_stat_t *stat = nullptr);

extern trx_undo_rec_t *trx_erase_fetch_next_rec(
    trx_id_t *modifier_trx_id,
    /*!< out: modifier trx id. this is the
    trx that created the undo record. */
    roll_ptr_t *roll_ptr,   /*!< out: roll pointer to undo record */
    ulint *n_pages_handled, /*!< in/out: number of UNDO log pages
                            handled */
    bool *last_log_rec,     /*!< out: last log record */
    mem_heap_t *heap);      /*!< in: memory heap where copied */

/** This function runs a purge batch.
@param[in]      n_purge_threads  number of purge threads
@param[in]      batch_size       number of pages to purge
@param[out]     next_sp_log      should truncate current sp log so that next
                                 sp log can be choosen.
@return number of undo log pages handled in the batch */
extern ulint trx_erase_attach_undo_recs(const ulint n_purge_threads,
                                        ulint batch_size, bool *next_sp_log);

extern ulint trx_erase(ulint n_purge_threads, ulint batch_size, bool truncate);

/** address of its sp list node.
 @return true if erase_sys_t::limit <= erase_sys_t::iter */
static inline bool trx_erase_check_limit() {
  /* limit is used to track till what point purge element has been
  processed and so limit <= iter.
  undo_no ordering is enforced only within the same rollback segment.
  If a transaction uses multiple rollback segments then we need to
  consider the rollback segment space id too. */

  return (erase_sys->iter.ommt.scn > erase_sys->limit.ommt.scn ||
          (erase_sys->iter.ommt.scn == erase_sys->limit.ommt.scn &&
           ((erase_sys->iter.undo_no >= erase_sys->limit.undo_no) ||
            (erase_sys->iter.undo_rseg_space !=
             erase_sys->limit.undo_rseg_space))));
}

/** return true if sp list is empty. */
extern bool trx_erase_sp_list_is_empty(trx_rseg_t *rseg);

/**
  precheck if txn of the row is erased, without really reading txn

  @param[in]    txn_rec     the current row to be checked

  @retval       bool        true if the corresponding txn has been erased
*/
bool precheck_if_txn_is_erased(const txn_rec_t *txn_rec);

}  // namespace lizard

#endif
