# include/wait_innodb_all_erased.inc
#
# SUMMARY
#

#    Waits until erased all undo records of innodb, or operation times out.
#
# USAGE
#
#    --source include/wait_innodb_all_erased.inc
#


--let $is_debug= `select (version() like '%debug%') as debug`

if ($is_debug)
{
--disable_query_log
set global innodb_purge_rseg_truncate_frequency = 1;
--enable_query_log
--source include/wait_innodb_all_purged.inc
--disable_query_log
let $trunc_trx_age = `SELECT VARIABLE_VALUE FROM performance_schema.global_status
WHERE VARIABLE_NAME = 'innodb_purge_done_scn';`;

let $wait_counter_init= 300;
if ($wait_timeout)
{
  let $wait_counter_init= `SELECT $wait_timeout * 10`;
}
# Reset $wait_timeout so that its value won't be used on subsequent
# calls, and default will be used instead.
let $wait_timeout= 0;

let $wait_counter= $wait_counter_init;

while ($wait_counter)
{
    let $erase_trx_age = `SELECT VARIABLE_VALUE FROM performance_schema.global_status
WHERE VARIABLE_NAME = 'innodb_erase_done_scn';`;

    let $success= `SELECT $trunc_trx_age <= $erase_trx_age`;
    if ($success)
    {
        let $wait_counter= 0;
    }
    if (!$success)
    {
        set global innodb_purge_run_now=ON;
        sleep 0.1;
        dec $wait_counter;
    }
}
if (!$success)
{
  die Timeout in wait_innodb_all_erased.inc;
}

set global innodb_purge_rseg_truncate_frequency = default;

--enable_query_log
}
