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

/** @file dd/lizard_dd_table.cc
 Lizard operations related to dd::Table.

 Created 2024-05-16 by Ting Yuan
 *******************************************************/

#include "lizard_dd_table.h"
#include "sql/dd/properties.h"

namespace lizard {

/** Set flashback_area into option of dd::Table.
 *
 * @param[in/out]	dd::Table
 * @param[in]	    is_dd_table
 */
void dd_table_set_flashback_area(dd::Table *table, bool is_dd_table) {
  bool val = false;

  if (!table->is_temporary() && !is_dd_table) {
    val = true;
  }

  table->options().set("flashback_area", val);
}

/** Retrieve flashback_area from dd::table options.
 *
 * @param[in]	dd::Table
 * @retval	true if flashback_area is set.
 */
template <typename Table>
bool dd_table_get_flashback_area(const Table &table) {
  bool val = false;

  const dd::Properties &options = table.options();
  if (options.exists("flashback_area")) {
    options.get("flashback_area", &val);
  }

  return val;
}

template bool dd_table_get_flashback_area<dd::Table>(const dd::Table &);
template bool dd_table_get_flashback_area<dd::Partition>(const dd::Partition &);

}  // namespace lizard
