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

/** @file include/lizard0dd0policy.cc
 Lizard DDL policy

 Created 2024-07-11 by Jiyang.Zhang
 *******************************************************/
#ifndef LIZARD_DD_POLICY_INCLUDED
#define LIZARD_DD_POLICY_INCLUDED

#include "sql/dd/lizard_policy_types.h"

namespace lizard {

extern bool validate_dd_index_policy(dd::Properties *options,
                                     const dict_index_t *index,
                                     const lizard::Ha_ddl_policy *ddl_policy);
/**
  Fill Indexes_policy from dd::Table or dd::Partition object.
  Which should be called when open a table.
*/
template <typename Table>
extern Indexes_policy dd_fill_indexes_policy(const Table *dd_table);

extern const Table_policy ha_ddl_create_table_policy(Ha_ddl_policy *ddl_policy,
                                                     const dict_table_t *table);

extern const Index_policy ha_ddl_create_index_policy(Ha_ddl_policy *ddl_policy,
                                                     const dict_table_t *table,
                                                     const dict_index_t *index);
}

#endif
