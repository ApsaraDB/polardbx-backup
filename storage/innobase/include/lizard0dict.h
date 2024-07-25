/*****************************************************************************

Copyright (c) 2013, 2020, Alibaba and/or its affiliates. All Rights Reserved.

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

/** @file include/lizard0dict.h
 Special Lizard tablespace definition.

 Created 2020-03-19 by Jianwei.zhao
 *******************************************************/

#ifndef lizard0dict_h
#define lizard0dict_h

#include "sql/dd/object_id.h"
#include "sql/package/gpp_stat.h"

#include "ddl0ddl.h"
#include "univ.i"
#include "api0api.h"
#include "dict0dict.h"
#include "mem0mem.h"

#include "lizard0txn0types.h"
#include "lizard0undo0types.h"
#include "lizard0dd0policy.h"

struct dict_table_t;
struct dict_index_t;

namespace dd {
class Table;
class Index;
}  // namespace dd

namespace lizard {

class Snapshot_vision;

/** Here define the lizard related dictionary content */

struct dict_lizard {
  /** The lizard tablespace space id */
  static constexpr space_id_t s_lizard_space_id = 0xFFFFFFFA;

  /** The dd::Tablespace::id of innodb_lizard */
  static constexpr dd::Object_id s_dd_lizard_space_id = 3;

  /** Whether the last file of lizard space is auto extend */
  static constexpr const bool s_file_auto_extend = true;

  /** The init file size (bytes) */
  static constexpr const ulint s_file_init_size = 12 * 1024 * 1024;

  /** The last file max size (bytes) */
  static constexpr const ulint s_last_file_max_size = 0;

  /** Lizard tablespace file count */
  static constexpr const ulint s_n_files = 1;

  /** The space name of lizard */
  static const char *s_lizard_space_name;

  /** The file name of lizard space */
  static const char *s_lizard_space_file_name;

  /** The lizard transaction tablespace is from max undo tablespace */
  static constexpr space_id_t s_max_txn_space_id =
      dict_sys_t::s_max_undo_space_id;

  /** The lizard transaction tablespace minmum space id */
  static constexpr space_id_t s_min_txn_space_id =
      dict_sys_t::s_max_undo_space_id - FSP_IMPLICIT_TXN_TABLESPACES + 1;

  /** Lizard: First several undo tbs will be treated as txn tablespace */
  static const char *s_default_txn_space_names[FSP_IMPLICIT_TXN_TABLESPACES];
};

/** Judge the undo tablespace is txn tablespace by name */
extern bool is_txn_space_by_name(const char *name);

inline bool is_lizard_column(const char *col_name) {
  ut_ad(col_name != nullptr);
  return (strncmp(col_name, "DB_SCN_ID", 9) == 0 ||
          strncmp(col_name, "DB_UNDO_PTR", 11) == 0 ||
          strncmp(col_name, "DB_GCN_ID", 9) == 0);
}

/**
  Add the SCN and UBA column into dict_table_t, for example:
  dict_table_t::col_names "...DB_SCN_ID\0DATA_UNDO_PTR\0..."

  @param[in]      table       dict_table_t
  @param[in]      heap        memory slice
*/
void dict_table_add_lizard_columns(dict_table_t *table, mem_heap_t *heap);

/**
  Add the lizard columns into data dictionary in server

  @param[in,out]	dd_table	data dictionary cache object
  @param[in,out]  primary   data dictionary primary key
*/
void dd_add_lizard_columns(dd::Table *dd_table, dd::Index *primary);

/** Add virtual GPP_NO column on index as virtual column.
 *
 * @param[in/out]	index
 * @param[in]		table
 * */
void dict_index_add_virtual_gcol(dict_index_t *index,
                                 const dict_table_t *table);

/** Add stored GPP_NO column on secondary index following PK Columns.
 *
 * @param[in/out]	new index.
 * @param[in]		index.
 * @param[in]		dictionary table
 */
void dict_index_add_stored_gcol(dict_index_t *new_index,
                                const dict_index_t *index,
                                const dict_table_t *table);

/**
  Init txn_desc with the creator trx when created.

  @param[in]      index       the index being created
  @param[in]      trx         creator transaction
  @return         DB_SUCCESS  Success
*/
dberr_t dd_index_init_txn_desc(dict_index_t *index, trx_t *trx);

/**
  Fill index txn information by from se_private_data.

  @param[in,out]  index       the index being opened.
  @param[in]      p           se_private_data from the mysql.indexes record.
  @return         true if failed
*/
bool dd_index_fill_txn_desc(dict_index_t *index, const dd::Properties &p);

/**
  Check if a index should be seen by a transaction.

  @param[in]      index       the index being opened.
  @param[in]      trx         transaction.
  @return         true if visible
*/
bool dd_index_modificatsion_visible(dict_index_t *index, const trx_t *trx,
                                    lizard::Snapshot_vision *snapshot_vision);

/**
 * Return prefined dict_table_t GPP_NO column.
 *
 * @return	always valid column. */
dict_col_t *dict_table_get_v_gcol(const dict_table_t *table);

/**
 * Copy column definition
 *
 * @param[in/out]	tuple
 * @Param[in]		dict_table_t */
void dict_table_copy_g_types(dtuple_t *tuple, const dict_table_t *table);

/** Collect gpp stats of each index. */
class Collector {
 public:
  Collector(std::vector<im::Index_gpp_stat> &stats) : m_stats(stats) {}

  Collector() = delete;

  /** Visitor.
  @param[in]      table   table to visit */
  void operator()(dict_table_t *table) {
    for (dict_index_t *index : table->indexes) {
      if (index->n_s_gfields == 0) {
        continue;
      }

      ut_ad(index->is_gstored());

      m_stats.emplace_back(im::Index_gpp_stat{table->name.m_name,
                                              (const char *)index->name,
                                              index->gpp_hit, index->gpp_miss});
    }
  }

 private:
  std::vector<im::Index_gpp_stat> &m_stats;
};

class Gpp_index_stat_flusher {
 public:
  Gpp_index_stat_flusher() = default;
  void operator()(dict_table_t *table) {
    for (dict_index_t *index : table->indexes) {
      if (index->n_s_gfields == 0) {
        continue;
      }

      ut_ad(index->is_gstored());
      index->gpp_hit.reset();
      index->gpp_miss.reset();
    }
  }
};

/**
  Get ordered field number from sec index for row log.
  @param[in]      index     dict_index_t
  @retval ordered field number
*/
ulint row_log_dict_index_get_ordered_n_fields(const dict_index_t *index);

/**
  Instantiate index related metadata about GPP...
  @param[in]      index_policy  index config info about GPP
  @param[in]      table         InnoDB table definition
  @param[in,out]  index         dict_index_t to fill
*/
extern void dd_fill_dict_index_format(const Index_policy &index_policy,
                                      const dict_table_t *table,
                                      dict_index_t *index);

/**
  Write lizard metadata of a index to dd::Index for regular table or
  dd::Partition_index for partition tables.

  @param[in,out]  dd_options  dd_options which might carry GPP info
  @param[in]      index           InnoDB index object
*/
extern void dd_write_index_format(dd::Properties *dd_options,
                                  const dict_index_t *index,
                                  const Ha_ddl_policy *ddl_policy);

/**
  Copy lizard metadata of a index to dd::Index for regular table or
  dd::Partition_index for partition tables.

  @param[out]     new_dd_options  new dd options to be modified.
  @param[in]      old_dd_options  old dd options from DD
*/
extern void dd_copy_index_format(dd::Properties &new_dd_options,
                                 const dd::Properties &old_dd_options);

/**
  Exchange IFT option between Partition Index and Swap Index.
  @param[in,out]   part_dd_options  dd options from dd::Partition_index
  @param[in,out]   swap_dd_options  dd options from dd::Index from Swap Table
*/
extern void dd_exchange_index_format(dd::Properties &part_dd_options,
                                     dd::Properties &swap_dd_options);

#if defined UNIV_DEBUG || defined LIZARD_DEBUG
/**
  Check the dict_table_t object

  @param[in]      table       dict_table_t

  @return         true        Success
*/
bool lizard_dict_table_check(const dict_table_t *table);

/**
  Check the dict_incex object

  @param[in]      index       dict_index_t
  @param[in]      check_table false if cannot check table

  @return         true        Success
*/
bool lizard_dict_index_check(const dict_index_t *index,
                             bool check_table = true);

/**
  Validate IFT option from dd::Index or dd::Partition_index is equal to infos in
  dict_index_t.

  @param[in]      dd_options  dd_options from dd::Index or dd::Partition_index
  @param[in]      index       InnoDB index object
  @return false if totally equal, true if not equal.
*/
extern bool validate_dd_index_format_match(const dd::Properties &options,
                                           const dict_index_t *index);

#endif /* UNIV_DEBUG || LIZARD_DEBUG define */

}  // namespace lizard

#if defined UNIV_DEBUG || defined LIZARD_DEBUG
#define assert_lizard_dict_table_check(table)     \
  do {                                            \
    ut_a(lizard::lizard_dict_table_check(table)); \
  } while (0)

#define assert_lizard_dict_index_check(index)     \
  do {                                            \
    ut_a(lizard::lizard_dict_index_check(index)); \
  } while (0)

#define assert_lizard_dict_index_check_no_check_table(index) \
  do {                                                       \
    ut_a(lizard::lizard_dict_index_check(index, false));     \
  } while (0)

/* Assert the n_s_gfields and gpp_stored are matched. */
#define assert_lizard_dict_index_gstored_check(index)         \
  do {                                                        \
    ut_ad((index->n_s_gfields > 0 && index->is_gstored()) ||  \
          (index->n_s_gfields == 0 && !index->is_gstored())); \
  } while (0)

#else

#define assert_lizard_dict_table_check(table)
#define assert_lizard_dict_index_check(index)
#define assert_lizard_dict_index_check_no_check_table(index)
#define assert_lizard_dict_index_gstored_check(index)

#endif /* UNIV_DEBUG || lizard_DEBUG define */

#endif  // lizard0dict_h
