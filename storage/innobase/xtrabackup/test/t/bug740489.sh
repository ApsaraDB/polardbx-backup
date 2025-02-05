############################################################################
# Bug #740489: Add --defaults-extra-file param to innobackupex
############################################################################
. inc/common.sh

start_server --innodb_file_per_table
load_sakila

run_cmd ${MYSQL} ${MYSQL_ARGS} <<EOF
SET PASSWORD FOR 'root'@'localhost' = 'password';
EOF

defaults_extra_file=$topdir/740489.cnf

cp $MYSQLD_VARDIR/my.cnf $defaults_extra_file

cat >> $defaults_extra_file <<EOF
[client]
password=password
EOF

backup_dir=$topdir/backup
run_cmd $XB_BIN \
    --defaults-extra-file=$defaults_extra_file --port=${MYSQLD_PORT} --socket=${MYSQLD_SOCKET} \
    --no-version-check --backup --target-dir=$backup_dir --skip-flush-binlog
vlog "Backup created in directory $backup_dir"

run_cmd ${MYSQL} ${MYSQL_ARGS} --password=password <<EOF
SET PASSWORD FOR 'root'@'localhost' = '';
EOF

stop_server
# Remove datadir
rm -r $mysql_datadir

# Restore sakila
vlog "Applying log"
vlog "###########"
vlog "# PREPARE #"
vlog "###########"
xtrabackup --prepare --target-dir=$backup_dir

vlog "Restoring MySQL datadir"
mkdir -p $mysql_datadir
vlog "###########"
vlog "# RESTORE #"
vlog "###########"
xtrabackup --copy-back --target-dir=$backup_dir

start_server
# Check sakila
run_cmd ${MYSQL} ${MYSQL_ARGS} --password=password -e "SELECT count(*) from actor" sakila
stop_server

rm -f $defaults_extra_file
