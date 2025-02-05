################################################################################
# Bug #1037379: SQL_THREAD left in stopped state of safe-slave-backup-timeout is
#               reached
# Bug #887803: innobackupex --safe-slave-backup-timeout option doesn't work
################################################################################

# Galera supports only RBR which does not replicated temp. tables
is_galera && skip_test "Requires a server without Galera support"

. inc/common.sh

skip_test "Requires master-slave mode, but xdb can not"

################################################################################
# Create a temporary table and pause "indefinitely" to keep the connection open
################################################################################
function create_temp_table()
{
    switch_server $master_id
    run_cmd $MYSQL $MYSQL_ARGS test <<EOF
CREATE TEMPORARY TABLE tmp(a INT);
INSERT INTO tmp VALUES (1);
SELECT SLEEP(10000);
EOF
}

master_id=1
slave_id=2

start_server_with_id $master_id --binlog-format=statement
start_server_with_id $slave_id --binlog-format=statement

setup_slave $slave_id $master_id

create_temp_table &
job_master=$!

sync_slave_with_master $slave_id $master_id

switch_server $slave_id

################################################################################
# First check if the SQL thread is left in the running state
# if it is running when taking a backup
################################################################################

# The following will fail due to a timeout
run_cmd_expect_failure $XB_BIN $XB_ARGS --backup --safe-slave-backup \
    --safe-slave-backup-timeout=3 --target-dir=$topdir/backup1

# Check that the SQL thread is running
run_cmd $MYSQL $MYSQL_ARGS -e "SHOW SLAVE STATUS\G" |
  egrep 'Slave_SQL_Running:[[:space:]]+Yes'

################################################################################
# Now check if the SQL thread is left in the stopped state
# if it is stopped when taking a backup
################################################################################

run_cmd $MYSQL $MYSQL_ARGS -e "STOP SLAVE SQL_THREAD"

# The following will fail due to a timeout
run_cmd_expect_failure $XB_BIN $XB_ARGS --backup --safe-slave-backup \
    --safe-slave-backup-timeout=3 --target-dir=$topdir/backup2

# Check that the SQL thread is stopped
run_cmd $MYSQL $MYSQL_ARGS -e "SHOW SLAVE STATUS\G" |
  egrep 'Slave_SQL_Running:[[:space:]]+No'

kill -SIGKILL $job_master
