##############################################################################
# Bug #1168513: innobackupex uses SHOW MASTER STATUS to obtain binlog file and
#               position
##############################################################################

. inc/common.sh

skip_test "Requires skip-log-bin, but xdb can not"

# Make sure we start the server with the binary log disabled
start_server --skip-log-bin

xtrabackup --backup --target-dir=$topdir/backup

# Check that to xtrabackup_binlog_info file is created
if [ -f $topdir/backup/xtrabackup_binlog_info ]
then
    vlog "$topdir/backup/xtrabackup_binlog_info should not be created!"
    exit -1
fi
