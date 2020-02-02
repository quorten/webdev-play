/* Get dates for auto-pay between given date range.  */

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef enum { false, true } bool;

bool update_monthly_auto_pay (struct tm *start, struct tm *end);
bool update_interval_auto_pay (time_t payday1, time_t pay_interval,
			       time_t start_time, time_t end_time);

int
main (int argc, char *argv[])
{
  struct tm cal_time1;
  struct tm cal_time2;
  time_t time1;
  time_t time2;

  struct tm cal_payday1;
  time_t payday1;

  if (argc != 3)
    return 1;

  /* TODO FIXME: Update dumb_cgipm and my house logs for dummies Perl
     scripts to not use %F %T, it is a late standard.  */
  memset (&cal_time1, 0, sizeof(struct tm));
  memset (&cal_time2, 0, sizeof(struct tm));
  strptime (argv[1], "%Y-%m-%d", &cal_time1);
  strptime (argv[2], "%Y-%m-%d", &cal_time2);
  time1 = mktime (&cal_time1);
  time2 = mktime (&cal_time2);

  printf ("Date 1: %04d-%02d-%02d\n",
	  cal_time1.tm_year + 1900, cal_time1.tm_mon + 1, cal_time1.tm_mday);
  printf ("Date 2: %04d-%02d-%02d\n",
	  cal_time2.tm_year + 1900, cal_time2.tm_mon + 1, cal_time2.tm_mday);
  /* Date range includes both first day, and last day given.  */

  printf ("\nMonthly paydates:\n");

  update_monthly_auto_pay (&cal_time1, &cal_time2);

  printf ("\nWeekly paydates:\n");

  /* Finding week pay dates?  A first pay-date is given.  Then you
     create a pay date for every N weeks after that first date.  This
     means we must do date conversions.  */

  memset (&cal_payday1, 0, sizeof(struct tm));
  cal_payday1.tm_year = 2019 - 1900;
  cal_payday1.tm_mon = 01 - 1;
  cal_payday1.tm_mday = 01;
  cal_payday1.tm_hour = 0;
  cal_payday1.tm_min = 0;
  cal_payday1.tm_sec = 0;
  payday1 = mktime (&cal_payday1);

  update_interval_auto_pay (payday1, 7 * 24 * 60 * 60,
			    time1, time2);

  return 0;
}

/* So now that we have some code whipped up to compute the auto pay
   dates, we simply need to plug it into the API for creating
   auto-transactions.  And we simply need some simple code for load
   and save data formats.  And simple code for lock/unlock files.  */

/* Generate monthly auto-pay transactions for the given time interval,
   inclusive.  Payment is given on the first day of each month.  */
/* TODO: We also need to pass through the data structure needed to
   complete the auto-pay transaction.  */
bool
update_monthly_auto_pay (struct tm *start, struct tm *end)
{
  unsigned mon_payday = 1; /* pay day in the month */

  unsigned year1 = start->tm_year;
  unsigned month1 = start->tm_mon;
  unsigned day1 = start->tm_mday;
  unsigned year2 = end->tm_year;
  unsigned month2 = end->tm_mon;
  unsigned day2 = end->tm_mday;
  unsigned i;

  /* Finding month pay dates in date range is easy.  Just iterate
     through the list of months, and select the day of the month of
     the pay date, that is the first day of the month.  */
  for (i = start->tm_year; i <= year2; i++) {
    unsigned j;
    unsigned last_month;
    if (i == year1)
      j = month1;
    else
      j = 0;
    if (i == year2)
      last_month = month2;
    else
      last_month = 11;
    for (; j <= last_month; j++) {
      if (i == year1 && j == month1 && mon_payday < day1)
	continue;
      /* TODO: Execute transaction creation API here.  */
      printf ("Pay date %04d-%02d-%02d\n", i + 1900, j + 1, mon_payday);
    }
  }

  return true;
}

/* Generate constant time interval auto-pay transactions for the given
   time interval, inclusive, using the given pay-day #1 to determine
   what day of the week to use.  Payment is given on the first day of
   each month.  */
/* TODO: We also need to pass through the data structure needed to
   complete the auto-pay transaction.  */
bool
update_interval_auto_pay (time_t payday1, time_t pay_interval,
			  time_t start_time, time_t end_time)
{
  time_t mod_interval;
  time_t cur_time;
  struct tm cal_time;

  if (start_time < payday1)
    start_time = payday1;

  mod_interval = (start_time - payday1) % pay_interval;
  cur_time = start_time;

  if (mod_interval != 0)
    cur_time += pay_interval - mod_interval;

  while (cur_time <= end_time) {
    localtime_r (&cur_time, &cal_time);
    /* TODO: Execute transaction creation API here.  */
    printf ("Pay date %04d-%02d-%02d\n",
	    cal_time.tm_year + 1900, cal_time.tm_mon + 1, cal_time.tm_mday);
    cur_time += pay_interval;
  }

  return true;
}
