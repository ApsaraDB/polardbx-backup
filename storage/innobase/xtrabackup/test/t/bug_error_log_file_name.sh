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

