### Purpose ###
#
# We check that the result of gpp stat is correct
#
#
### Parameters ###
#
# $SELECT_STATEMENTS        SQL for producing gpp hit/miss.
# $HIT_TIMES                Times of hit.
# $MISS_TIMES               Times of miss.
# $HIT_MISS_TIMES           Times of hit/miss.

call dbms_stat.flush_gpp();

--disable_result_log
--eval $SELECT_STATEMENTS
--enable_result_log

--let $cur_scan_guess_hit= query_get_value(show status like "Lizard_index_scan_guess_clust_hit", Value, 1)
--let $cur_scan_guess_miss=query_get_value(show status like "Lizard_index_scan_guess_clust_miss", Value, 1)

if ($HIT_MISS_TIMES!='')
{
  --let $assert_text= gpp hit/miss times shoule be $HIT_MISS_TIMES
  --let $assert_cond= $cur_scan_guess_hit + $cur_scan_guess_miss = $HIT_MISS_TIMES
  --source include/assert.inc
}

if ($HIT_TIMES!='')
{
  --let $assert_text= gpp hit times shoule be $HIT_TIMES
  --let $assert_cond= $cur_scan_guess_hit = $HIT_TIMES
  --source include/assert.inc
}

if ($MISS_TIMES!='')
{
  --let $assert_text= gpp miss times shoule be $MISS_TIMES
  --let $assert_cond= $cur_scan_guess_miss = $MISS_TIMES
  --source include/assert.inc
}
