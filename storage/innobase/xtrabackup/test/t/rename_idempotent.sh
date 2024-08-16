start_server

require_server_version_higher_than 8.0.0

function stop_checkpoint() {
  mysql test -e "set global innodb_redo_log_capacity=500*1024*1024"
  mysql test -e "SET GLOBAL innodb_log_checkpoint_now = 1"
  mysql test -e "SET GLOBAL innodb_page_cleaner_disabled_debug = 1"
  mysql test -e "SET GLOBAL innodb_checkpoint_disabled = 1"
}

# TEST 1
# checkpoint
# t1[1] --> t2[1]
# t2[1] --> t3[1]
# create t2[2]
# XB backup t2[2] t3[1], and try to replay "t1[1] --> t2[1]"

mysql test -e "create table t1 (id int) engine = innodb"
mysql test -e "insert into t1 values (1)"

stop_checkpoint

mysql test -e "rename table t1 to t2"

mysql test -e "rename table t2 to t3"

mysql test -e "create table t2 (id int) engine = innodb"

mysql test -e "insert into t3 values (1)"

checksum_a=`checksum_table test t3`
vlog "Checksum before is $checksum_a"

xtrabackup --backup --target-dir=$topdir/backup

xtrabackup --prepare --target-dir=$topdir/backup

stop_server

rm -rf $mysql_datadir

xtrabackup --copy-back --target-dir=$topdir/backup

start_server

checksum_b=`checksum_table test t3`
vlog "Checksum after is $checksum_b"
if [ "$checksum_a" -eq "$checksum_b" ]
then
  vlog "Checksums are Ok"
else
  vlog "Checksums are not equal"
  exit -1
fi

