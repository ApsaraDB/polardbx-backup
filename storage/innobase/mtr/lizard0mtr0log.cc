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

/** @file mtr/lizard0mtr0log.cc
   Lizard Mini-transaction logging


 Created 2024-04-30 by Yichang Song
 *******************************************************/

#include "lizard0mtr0log.h"
#include "dict0mem.h"
#include "lizard0dict.h"
#include "mtr0log.h"

namespace lizard {

/** Get the sec_lfields_extra_flag from index.
  @param[in]  index          B-tree index.
  @param[out] sl_extra_flag  secondary index lizard fields extra flag
  @param[out] n_sec_lfields  number of secondary index lizard fields
*/
void get_sec_lfields_extra_flag(const dict_index_t *index,
                                uint8_t &sl_extra_flag,
                                uint8_t &n_sec_lfields) {
  sl_extra_flag = 0;
  n_sec_lfields = 0;
  assert_lizard_dict_index_gstored_check(index);

  if (index->n_s_gfields > 0) {
    ut_ad(index->n_s_gfields == 1);
    sl_extra_flag |= SEC_LFIELDS_EXTRA_GPP_FLAG;
    n_sec_lfields++;
  }
}

/** Log the sec_lfields_extra_flag.
  @param[in]     flag           1 byte flag indicating whether to log
                                sl_extra_flag or not
  @param[in]     sl_extra_flag  1 byte secondary index lizard fields extra flag
                                to be logged
  @param[in,out] log_ptr        REDO LOG buffer pointer */
void log_index_sec_lfields_extra_flag(uint8_t flag, uint8_t sl_extra_flag,
                                      byte *&log_ptr) {
  ut_ad(sl_extra_flag || !IS_SEC_LFIELDS(flag));
  if (IS_SEC_LFIELDS(flag)) {
    mach_write_to_1(log_ptr, sl_extra_flag);
    log_ptr += 1;
  }
}

/** Parse the sec_lfields_extra_flag.
  @param[in]  flag           1 byte flag indicating whether to parse
                             sl_extra_flag or not
  @param[in]  ptr            pointer to buffer
  @param[in]  end_ptr        pointer to end of buffer
  @param[out] sl_extra_flag  read 1 bytes sec_lfields_extra_flag
  @param[out] n_sec_lfields  count number of secondary index lizard fields
  @return pointer to buffer. */
byte *parse_index_lfields_extra_flag(uint8_t flag, byte *ptr,
                                     const byte *end_ptr,
                                     uint8_t &sl_extra_flag,
                                     uint8_t &n_sec_lfields) {
  sl_extra_flag = 0;
  n_sec_lfields = 0;
  if (!IS_SEC_LFIELDS(flag)) {
    return ptr;
  }

  DBUG_EXECUTE_IF("crash_if_n_gfields_in_redo", DBUG_SUICIDE(););
  if (end_ptr < ptr + 1) {
    return (nullptr);
  }

  sl_extra_flag = mach_read_from_1(ptr);
  ptr += 1;
  if (sl_extra_flag & SEC_LFIELDS_EXTRA_GPP_FLAG)
    n_sec_lfields++;
  return ptr;
}
} // namespace lizard