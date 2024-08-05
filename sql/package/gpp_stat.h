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

/** @file sql/package/gpp_stat.h
 Result structure for index gpp stat.

 Created 2024-04-10 by Ting Yuan
 *******************************************************/

#ifndef SQL_GPP_STAT_INCLUDED
#define SQL_GPP_STAT_INCLUDED

#include <string>
namespace im {

/* Result structure for index gpp stat. */
typedef struct Index_gpp_stat {
  std::string table_name;
  std::string index_name;
  uint64_t gpp_hit;
  uint64_t gpp_miss;

 public:
  Index_gpp_stat(std::string table_name, std::string index_name,
                 uint64_t gpp_hit, uint64_t gpp_miss)
      : table_name(table_name),
        index_name(index_name),
        gpp_hit(gpp_hit),
        gpp_miss(gpp_miss) {}
} Index_gpp_stat;

} /* namespace im */

#endif
