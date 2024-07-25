### Purpose ###
#
# This inc file is invoked from lizard.gpp_no test case.
#
# Test various types of gpp_no in gpp reads
#
#
### Parameters ###
#
# $ROW_FORMAT

delimiter |;

# --------------------------------------------------------------------------

CREATE PROCEDURE iterate_pages_in_space()
BEGIN
  DECLARE v_iter INT DEFAULT 0;
  DECLARE space_size INT DEFAULT 0;
  SELECT FILE_SIZE/PAGE_SIZE INTO space_size FROM information_schema.INNODB_TABLESPACES WHERE NAME='test/t1';
  SELECT 'Space size of test/t1:', space_size;
  WHILE v_iter < space_size DO
    SET DEBUG='+d,set_gpp_enum_page_in_space';
    SET GLOBAL innodb_dbug_gpp_no = v_iter;
    SELECT @@innodb_dbug_gpp_no;
    SELECT id, c FROM t1 WHERE k = 4;
    show status like '%scan_guess%';
    SET v_iter = v_iter + 1;
  END WHILE;
END|

# --------------------------------------------------------------------------

delimiter ;|

--eval CREATE TABLE t1 ( id int NOT NULL, k int NOT NULL, c int NOT NULL, pad varchar(8192), PRIMARY KEY (id), KEY (k)) row_format=$ROW_FORMAT engine=innodb

set global innodb_lizard_stat_enabled=on;
set global innodb_purge_stop_now = on;
set global innodb_index_purge_guess_clust_enabled=on;
set global innodb_index_scan_guess_clust_enabled=on;

INSERT INTO t1 (id, k, c, pad) VALUES (1, 1, 1, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (2, 2, 2, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (3, 3, 3, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (4, 4, 4, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (5, 5, 5, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (6, 6, 6, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (7, 7, 7, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));
INSERT INTO t1 (id, k, c, pad) VALUES (8, 8, 8, LEFT(REPEAT('A', 4000), CHAR_LENGTH(REPEAT('A', 4000))));

select PAGE_NUMBER, PAGE_TYPE, TABLE_NAME, INDEX_NAME, NUMBER_RECORDS, DATA_SIZE from INFORMATION_SCHEMA.INNODB_BUFFER_PAGE WHERE TABLE_NAME like "%t1%" order by PAGE_NUMBER;

connect (con1_$ROW_FORMAT,localhost,root,,);
--echo
--echo === Correct gpp ===
SELECT id, c FROM t1 WHERE k = 4;
show status like '%scan_guess%';

--echo
--echo === FIL NULL ===
SET DEBUG='+d,set_gpp_null';
SELECT id, c FROM t1 WHERE k = 4;
show status like '%scan_guess%';

--echo
--echo === Iterate pages in space  ===
CALL iterate_pages_in_space();

--echo
--echo === Freed page ===
connection default;
set global innodb_purge_run_now = on;
start transaction;
DELETE FROM t1 WHERE k = 1;
DELETE FROM t1 WHERE k = 2;
DELETE FROM t1 WHERE k = 3;
commit;

--source include/wait_innodb_all_purged.inc
show status like '%scan_guess%';

--echo
--echo The page (where page_no == 7), which originally contained the record with id = 4, has now been freed. 
select PAGE_NUMBER, PAGE_TYPE, TABLE_NAME, INDEX_NAME, NUMBER_RECORDS, DATA_SIZE from INFORMATION_SCHEMA.INNODB_BUFFER_PAGE WHERE TABLE_NAME like "%t1%" order by PAGE_NUMBER;

--echo
--echo Attempting to locate a clustered index record using an outdated gpp_no of a page that has already been freed.
SELECT id, c FROM t1 WHERE k = 4;
show status like '%scan_guess%';

--source include/kill_and_restart_mysqld.inc
set global innodb_lizard_stat_enabled=on;
set global innodb_index_purge_guess_clust_enabled=on;
set global innodb_index_scan_guess_clust_enabled=on;

--echo
--echo Re-examine the previously freed page following a server restart.
SELECT id, c FROM t1 WHERE k = 4;

--echo
--echo === Freed page in btr load ===
DELETE FROM t1 WHERE id < 8;
ALTER TABLE t1 DROP PRIMARY KEY, ADD PRIMARY KEY (c);

--echo 
--echo The page (where page_no == 6), previously allocated for a B-tree loading operation, has now been freed.
select PAGE_NUMBER, PAGE_TYPE, TABLE_NAME, INDEX_NAME, NUMBER_RECORDS from INFORMATION_SCHEMA.INNODB_BUFFER_PAGE 
where space = (select space from information_schema.innodb_indexes 
where table_id = (select table_id from information_schema.innodb_tables where name like '%t1%') limit 1) 
and page_type = 'index' and PAGE_NUMBER=6;

--echo
--echo Attempting to locate a clustered index record using an outdated gpp_no of a page that has already been freed.
SET DEBUG='+d,set_gpp_enum_page_in_space';
SET GLOBAL innodb_dbug_gpp_no = 6;
SELECT @@innodb_dbug_gpp_no;
SELECT id, k FROM t1 WHERE k = 8;
show status like '%scan_guess%';

--source include/kill_and_restart_mysqld.inc
set global innodb_lizard_stat_enabled=on;
set global innodb_index_purge_guess_clust_enabled=on;
set global innodb_index_scan_guess_clust_enabled=on;

--echo
--echo Re-examine the previously freed page following a server restart.
SET DEBUG='+d,set_gpp_enum_page_in_space';
SET GLOBAL innodb_dbug_gpp_no = 6;
SELECT @@innodb_dbug_gpp_no;
SELECT id, k FROM t1 WHERE k = 8;
show status like '%scan_guess%';

if ($ROW_FORMAT != COMPRESSED)
{
  # compressed table don't guess.
  --echo
  --echo # Set gpp_no to a large number which is out of tablespace.
  SET DEBUG='+d,set_gpp_enum_page_in_space';
  SET GLOBAL innodb_dbug_gpp_no = 999999;
  SELECT @@innodb_dbug_gpp_no;

  SET DEBUG_SYNC = 'gpp_ignore_missing_before_free_page SIGNAL s1 WAIT_FOR s2';
  --send SELECT id, k FROM t1 WHERE k = 8;

  connect (con2_$ROW_FORMAT,localhost,root,,);
  SET DEBUG='+d,set_gpp_enum_page_in_space';
  SET DEBUG_SYNC = 'now WAIT_FOR s1';
  # If the following sql is blocked by gpp_ignore_missing debug_sync, mysqld will crash.
  SET DEBUG_SYNC = 'gpp_hit_io_fix_read SIGNAL s2';
  SELECT id, k FROM t1 WHERE k = 8;
  show status like '%scan_guess%';

  connection default;
  reap;
  SET DEBUG='+d,set_gpp_enum_page_in_space';
  SET GLOBAL innodb_dbug_gpp_no = 999999;
  SELECT @@innodb_dbug_gpp_no;
  SET DEBUG_SYNC = 'gpp_ignore_missing_after_free_page SIGNAL s1 WAIT_FOR s2';
  --send SELECT id, k FROM t1 WHERE k = 8;

  connection con2_$ROW_FORMAT;
  SET DEBUG='+d,set_gpp_enum_page_in_space';
  SET DEBUG_SYNC = 'now WAIT_FOR s1';
  # If the following sql is blocked by gpp_ignore_missing debug_sync, mysqld will crash.
  SELECT id, k FROM t1 WHERE k = 8;
  show status like '%scan_guess%';
  SET DEBUG_SYNC = 'now signal s2';
  show status like '%scan_guess%';

  connection default;
  reap;

}

--echo 
--echo After the server restart, the page with page_no == 6 is now marked as allocated.
select PAGE_NUMBER, PAGE_TYPE, TABLE_NAME, INDEX_NAME, NUMBER_RECORDS, DATA_SIZE from INFORMATION_SCHEMA.INNODB_BUFFER_PAGE 
where space = (select space from information_schema.innodb_indexes 
where table_id = (select table_id from information_schema.innodb_tables where name like '%t1%') limit 1) 
and (page_type = 'index' or page_number=6) order by PAGE_NUMBER;

connection default;
drop table t1;
set global innodb_lizard_stat_enabled = default;
set global innodb_purge_stop_now = default;
set global innodb_purge_run_now = default;
set global innodb_dbug_gpp_no = default;
drop procedure iterate_pages_in_space;

--source include/kill_and_restart_mysqld.inc
