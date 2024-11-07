# Copyright (c) 2023, 2024, Alibaba and/or its affiliates. All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License, version 2.0, as published by the
# Free Software Foundation.
# 
# This program is also distributed with certain software (including but not
# limited to OpenSSL) that is licensed under separate terms, as designated in a
# particular file or component or in included license documentation. The authors
# of MySQL hereby grant you an additional permission to link the program and
# your derivative works with the separately licensed software that they have
# included with MySQL.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
# for more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
#

########################################################################
# [Fix] Prepare print 'Attempted to open a previously'
########################################################################

. inc/common.sh

start_server

backup_dir=$topdir/backup

run_cmd ${MYSQL} ${MYSQL_ARGS} -e "SET GLOBAL innodb_file_per_table=1"

load_dbase_schema sakila
load_dbase_data sakila

xtrabackup --backup --tables='actor' --target-dir=$backup_dir

stop_server

xtrabackup --prepare --export --target-dir=$backup_dir >${TEST_VAR_ROOT}/prepare.log 2>&1

if grep -c 'Attempted to open a previously opened tablespace. Previous tablespace' ${TEST_VAR_ROOT}/prepare.log 
then
  die "found error log like 'Attempted to open a previously' "
fi

