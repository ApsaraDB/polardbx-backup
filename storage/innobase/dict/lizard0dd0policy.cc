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

/** @file dict/lizard0dd0policy.cc
 Lizard DDL policy

 Created 2024-07-11 by Jiyang.Zhang
 *******************************************************/

#include "sql/dd/types/partition.h"
#include "sql/dd/types/partition_index.h"
#include "sql/dd/types/table.h"
#include "sql/sql_class.h"

#include "dict0dict.h"
#include "dict0mem.h"
#include "ut0dbg.h"

#include "lizard0dd0policy.h"

namespace lizard {

static bool can_gpp(const dict_table_t *table, const dict_index_t *index) {
  return !table->is_compressed() && !table->is_temporary() &&
         !table->is_intrinsic() && !dict_sys_t::is_dd_table_id(table->id) &&
         !table->is_system_table && !dict_sys->is_permanent_table(table) &&
         !(index->type & DICT_IBUF) && !(index->type & DICT_SDI) &&
         !(index->type & DICT_FTS) && !dict_index_is_spatial(index) &&
         !index->is_clustered();
}

static bool can_fba(const dict_table_t *table) {
  return !table->is_temporary() && !table->is_intrinsic() &&
         !dict_sys_t::is_dd_table_id(table->id) && !table->is_system_table &&
         !dict_sys->is_permanent_table(table);
}

void Index_policy::create(Ha_ddl_policy *ddl_policy, const dict_table_t *table,
                          const dict_index_t *index) {
  ut_ad(!m_gpp);
  ut_a(m_inited == false);
  m_inited = true;

  if (!ddl_policy) {
    return;
  }

  if (ddl_policy->hint_gpp()) {
    ut_ad(!(index->type & DICT_SDI));
    ut_ad(!(index->type & DICT_IBUF));

    m_gpp = can_gpp(table, index);
  }
}

void Index_policy::restore(const dd::Properties &options) {
  ulonglong format = 0;

  if (options.exists(OPTION_IFT)) {
    options.get(OPTION_IFT, &format);
  }

  m_gpp = (format & IFT_GPP);

  m_inited = true;
}

void Table_policy::create(Ha_ddl_policy *ddl_policy,
                          const dict_table_t *table) {
  ut_a(m_inited == false);
  m_inited = true;

  if (!ddl_policy) {
    return;
  }

  if (ddl_policy->hint_fba()) {
    m_flashback_area = can_fba(table);
  }
}

void Table_policy::restore(const dd::Properties &options) {
  /** TODO: fix it <12-07-24, zanye.zjy> */
}

const Index_policy ha_ddl_create_index_policy(Ha_ddl_policy *ddl_policy,
                                              const dict_table_t *table,
                                              const dict_index_t *index) {
  Index_policy index_policy;

  index_policy.create(ddl_policy, table, index);

  return index_policy;
}

const Table_policy ha_ddl_create_table_policy(Ha_ddl_policy *ddl_policy,
                                              const dict_table_t *table) {
  Table_policy table_policy;

  table_policy.create(ddl_policy, table);
  return table_policy;
}

Ha_ddl_policy::Ha_ddl_policy(const THD *thd) : m_hint_fba(0), m_hint_gpp(0) {
  if (thd->variables.opt_flashback_area) {
    ut_a(!m_hint_fba);
    m_hint_fba = 1;
  }

  if (thd->variables.opt_index_format_gpp_enabled) {
    ut_a(!m_hint_gpp);
    m_hint_gpp = 1;
  }
}

// static bool dd_index_options_has_ift(const dd::Properties *options) {
//   return (options->exists(OPTION_IFT));
// }

bool validate_dd_index_policy(dd::Properties *options,
                              const dict_index_t *index,
                              const lizard::Ha_ddl_policy *ddl_policy) {
  return true;
}

template <typename Table>
Indexes_policy dd_fill_indexes_policy(const Table *dd_table) {
  Indexes_policy indexes_policy;

  for (auto dd_index : dd_table->indexes()) {
    indexes_policy.emplace_back();
    indexes_policy.back().restore(dd_index->options());
  }

  return indexes_policy;
}

template Indexes_policy dd_fill_indexes_policy<dd::Table>(
    const dd::Table *dd_table);

template Indexes_policy dd_fill_indexes_policy<dd::Partition>(
    const dd::Partition *dd_table);

#ifdef UNIV_DEBUG
static bool dd_table_options_has_fba(const dd::Properties *options) {
  ulonglong format = 0;

  if (options->exists(OPTION_IFT)) {
    options->get(OPTION_IFT, &format);
    return (format & IFT_GPP);
  } else {
    return false;
  }
}
#endif
void dd_write_table_fba(dd::Properties *options, const dict_table_t *table,
                        const Ha_ddl_policy *ddl_policy) {
  ut_ad(!dd_table_options_has_fba(options));

  if (!table->is_2pp) {
    return;
  }

  options->set(TABLE_OPTION_FBA, table->is_2pp);
}
}  // namespace lizard
