/* My own implementation of a calendar time library.  Why?  As of the
   year 2019, the Linux community still has too much bad attitude,
   they refuse to implement 64-bit Unix time on 32-bit Linux.

   But, come on!  I have a Raspberry Pi, I have a toy banking system,
   and I want to forecast future autopay transactions past the year
   2038.  And, since I'm also building my own standard C library, I
   might as well make a fully Unix compatible implementation while I'm
   at it.  */

/* Okay, BETTER implementation idea!

How many leap years in the intermin?  Compute how many Gregorian
calendar cycles.  Then you know the number of leap years in between,
you can then add the number of extra seconds in a day to get correct
Unix time.

Compute the position in the Gregorian calendar cycles for each year.

gy1 = y1 - 1600;
gy1_div4 = gy1 / 4;
gy1_div100 = gy1_div4 / 25;
gy1_div400 = gy1_div100 / 4;

gy2 = y2 - 1600;
gy2_div4 = gy2 / 4;
gy2_div100 = gy2_div4 / 25;
gy2_div400 = gy2_div100 / 4;

So now you know the cycle numbers.  If they are all the same, then
there are definitely no leap years in between.

When they are different, you can compute the number of leap years in
between.

num_lyears = gy2_div4 - gy1_div4;
num_lyears -= gy2_div100 - gy1_div100;
num_lyears += gy2_div400 - gy1_div400;

It's as simple as that.  Wow!

Yeah, forward conversion is easy, but reverse conversion?  Don't you
need a table for that?  Convert Unix time to calendar time.
Nevertheless, you can start by computing the number of days elapsed.
Then you can definitely count the number of non-leap years in between.
If it's greater than 400, that's easy, you know there's one full
400-year cycle contained in there.

What's left over, you disambiguate one step at a time.  100 year
cycles, then 4 year cycles.  There you have it, you can compute
without lookup tables, then you can store a cache of computed results,
etc.

*/

typedef enum { false, true } bool;

typedef unsigned long ulong;

EA_TYPE(ulong);

/* Determine if the given year is a leap year on the Gregorian
   calendar.  */
bool
is_leap_year (long year)
{
  if (year % 4 == 0) {
    /* Check if it's divisible by 100.  */
    if (year % 100 == 0) {
      /* Only a leap year if it's divisible by 400.  */
      if (year % 400 == 0)
	return true;
      else
	return false;
    } else /* It's a leap year.  */
      return true;
  }
  /* else */
  return false;
}

/* Determine the number of days in the given month on the given year.
   January is month number zero.  `is_lyear' is the pre-computed
   determination of whether the target year is a leap year or not.
   Returns (unsigned char)-1 on invalid month.  */
unsigned char
days_in_month (bool is_lyear, unsigned char month)
{
  unsigned char days_months[12] =
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  unsigned char num_days;
  if (month >= 12)
    return (unsigned char)-1;
  num_days = days_months[month];
  if (month == 1 && is_lyear) /* February on leap year */
    num_days++;
  return num_days;
}

/* Now we define special single-step counting functions for the sake
   of efficiently converting to and from Unix time without modulo
   division.  */

struct LeapYearMods_tag
{
  long year;
  unsigned char mod4;
  /* ((year / 4) % 25 == 0) equivalent to (year % 100 == 0) */
  unsigned char div4_mod25;
  /* ((year / 100) % 25 == 0) equivalent to (year % 400 == 0) */
  unsigned char div100_mod4;
};
typedef struct LeapYearMods_tag LeapYearMods;

/* Initialize modulo-year and modulo day of the week */
void
init_mod_year (LeapYearMods *mods, long year)
{
  if (mods == NULL)
    return;
  /* else */
  {
    long div4 = year / 4;
    char mod4 = year % 4;
    long div100 = div4 / 25;
    char div4_mod25 = div4 % 25;
    char div100_mod4 = div100 % 4;
    if (mod4 < 0)
      mod4 += 4;
    if (div4_mod25 < 0)
      div4_mod25 += 25;
    if (div100_mod4 < 0)
      div100_mod4 += 4;
    mods->year = year;
    mods->mod4 = mod4;
    mods->div4_mod25 = div4_mod25;
    mods->div100_mod4 = div100_mod4;
  }
}

/* Next modulo-year */
void
next_mod_year (LeapYearMods *mods)
{
  if (mods == NULL)
    return;
  /* else */
  {
    unsigned char mod4 = mods->mod4;
    unsigned char div4_mod25 = mods->div4_mod25;
    unsigned char div100_mod4 = mods->div100_mod4;
    mods->year++;
    mod4++;
    if (mod4 == 4) {
      mod4 = 0;
      div4_mod25++;
      if (div4_mod25 == 25) {
	div4_mod25 = 0;
	div100_mod4++;
	if (div100_mod4 == 4)
	  div100_mod4 = 0;
      }
    }
    mods->mod4 = mod4;
    mods->div4_mod25 = div4_mod25;
    mods->div100_mod4 = div100_mod4;
  }
}

/* Previous modulo-year */
void
prev_mod_year (LeapYearMods *mods)
{
  if (mods == NULL)
    return;
  /* else */
  {
    unsigned char mod4 = mods->mod4;
    unsigned char div4_mod25 = mods->div4_mod25;
    unsigned char div100_mod4 = mods->div100_mod4;
    mods->year--;
    if (mod4 == 0) {
      mod4 = 4;
      if (div4_mod25 == 0) {
	div4_mod25 = 25;
	if (div100_mod4 == 0)
	  div100_mod4 = 4;
	div100_mod4--;
      }
      div4_mod25--;
    }
    mod4--;
    mods->mod4 = mod4;
    mods->div4_mod25 = div4_mod25;
    mods->div100_mod4 = div100_mod4;
  }
}

/* Is modulo-year a leap year?  */
bool
is_leap_mod_year (LeapYearMods *mods)
{
  if (mods == NULL)
    return false;
  /* else */
  {
    unsigned char mod4 = mods->mod4;
    unsigned char div4_mod25 = mods->div4_mod25;
    unsigned char div100_mod4 = mods->div100_mod4;
    if (mod4 == 0) {
      /* Check if it's divisible by 100.  */
      if (div4_mod25 == 0) {
	/* Only a leap year if it's divisible by 400.  */
	if (div100_mod4 == 0)
	  return true;
	else
	  return false;
      } else /* It's a leap year.  */
	return true;
    }
    /* else */
    return false;
  }
}

#define SECS_PER_DAY (60 * 60 * 24)
#define DAYS_PER_REG_YEAR 365
#define DAYS_PER_LEAP_YEAR 366
#define SECS_PER_REG_YEAR (SECS_PER_DAY * DAYS_PER_REG_YEAR)
#define SECS_PER_LEAP_YEAR (SECS_PER_DAY * DAYS_PER_LEAP_YEAR)
/* Number of days/seconds in a typical 4-year cycle.  */
#define DAYS_4YEAR_CYC (DAYS_PER_REG_YEAR * 3 + DAYS_PER_LEAP_YEAR)
#define SECS_4YEAR_CYC (SECS_PER_REG_YEAR * 3 + SECS_PER_LEAP_YEAR)
/* Number of days/seconds in a typical 100-year cycle.  */
#define DAYS_100YEAR_CYC (DAYS_4YEAR_CYC * 24 + DAYS_PER_REG_YEAR * 4)
#define SECS_100YEAR_CYC (SECS_4YEAR_CYC * 24 + SECS_PER_REG_YEAR * 4)
/* Number of days/seconds in a 400-year cycle.  */
#define DAYS_400YEAR_CYC (DAYS_100YEAR_CYC * 3 + DAYS_4YEAR_CYC * 25)
#define SECS_400YEAR_CYC (SECS_100YEAR_CYC * 3 + SECS_4YEAR_CYC * 25)

/* Modulo day of the week.  Every regular year increases the day of
   the week by one (365 % 7 == 1), leap years increase the day of the
   week by two (366 % 7 == 2).  */
/* unsigned char mod_dotw; */ /* Range is 0 - 6, full days since Sunday */

/* Convert Unix time to calendar time.  */
/* How do we do this?  This is best done using a pre-computed lookup
   table over a designated date range.  But, if we don't know what the
   designated date range is, we can compute the lookup table as we go.
   Starting from the Unix epoch, we count forwards or backwards in
   years until we exceed the given Unix time.  Then, once we know the
   year, we can simply modulo-divide to determine the days, hours,
   minutes, and seconds.  The month can be determined from the number
   of days into the year.

   Day of the week???  January 1st, 1601 was a Monday.  January 1st,
   1970 was a Thursday.  Count up weekdays from there.

   When we have a lookup table, we can do a binary search to see if
   the given value is within range.  If it is beyond range, then we
   extend our lookup table to reach the new range.  Alternatively, and
   probably better, the number is truncated to a certain number of
   most significant bits, then that value is used to index into the
   lookup table.

     SECS_PER_REG_YEAR == 31536000
     31536000 > 16777216
     16777216 == 2^24

   So, if we truncate the bottom 24 bits, we get a lookup table where
   about three entries are taken up for one year.  And, this also
   tells us in a very punctual way, 32-bit Unix time left less than 8
   bits for the year number (approx. 256 / 3), repeating the same
   mistake of the pre-Y2K COBOL programmers.

   Our bit-wise lookup table includes the year number, which then
   points to another lookup indexed by year.  What key information
   must be included about the years?

   * The year (of course)
   * Is this a leap year?
   * The day of the week the year starts on
   * The Unix time epoch of the start of the year

   Matter of fact, now that I mention all of this, we might want a
   reprogrammable epoch date for the lookup table computation that can
   be specified at compile-time.

*/

struct YearLUTEntry_tag
{
  long year;
  unsigned char is_lyear;
  unsigned char start_dotw;
  long unix_time;
};
typedef struct YearLUTEntry_tag YearLUTEntry;

EA_TYPE(YearLUTEntry);

/* The epoch year itself.  That is, the "epoch" from which the year
   lookup tables are generated, which may be different from the Unix
   time epoch.  */
YearLUTEntry year_lut_epoch;

/* Years counting after the epoch.  The epoch is also included at
   zero.  */
YearLUTEntry_array year_lut_pos;
ulong_array bin_year_lut_pos;

/* Years counting before the epoch.  The epoch is also included at
   zero.  */
YearLUTEntry_array year_lut_neg;
ulong_array bin_year_lut_neg;

/* Extend the positive year lookup table until it can lookup the given
   Unix time.  */
void
extend_year_lut_pos (long unix_time)
{
  unsigned i = year_lut_pos.len - 1;
  LeapYearMods mods;
  init_mod_year (&mods, year_lut_pos.d[i].year);
  while (year_lut_pos.d[i].unix_time < unix_time) {
    i++;
    next_mod_year (&mods);
    year_lut_pos.d[i].year = mods.year;
    year_lut_pos.d[i].is_lyear = is_leap_mod_year (&mods);
    /* To compute the Unix time of THIS year, add the length in time
       of the LAST year.  */
    year_lut_pos.d[i].start_dotw = year_lut_pos.d[i-1].start_dotw + 1;
    year_lut_pos.d[i].unix_time =
      year_lut_pos.d[i-1].unix_time + SECS_PER_REG_YEAR;
    if (year_lut_pos.d[i-1].is_lyear) {
      year_lut_pos.d[i].start_dotw++;
      year_lut_pos.d[i].unix_time += SECS_PER_DAY;
    }
    if (year_lut_pos.d[i].start_dotw >= 7)
      year_lut_pos.d[i].start_dotw -= 7;
    EA_ADD(year_lut_pos);
    { /* Now loop to update the binary lookup table.  */
      long cur_time = (year_lut_pos.d[i-1].unix_time >> 24) + 1;
      long end_time = year_lut_pos.d[i].unix_time >> 24;
      /* Fill in the intermediate entries for the last year.  */
      while (cur_time < end_time) {
	EA_INSERT(bin_year_lut_pos, cur_time, i - 1);
	cur_time++;
      }
      /* Fill in the first entry for this year.  */
      EA_INSERT(bin_year_lut_pos, cur_time, i);
    }
  }
}

/* Extend the negative year lookup table until it can lookup the given
   Unix time.  */
void
extend_year_lut_neg (long unix_time)
{
  unsigned i = year_lut_neg.len - 1;
  LeapYearMods mods;
  init_mod_year (&mods, year_lut_neg.d[i].year);
  while (year_lut_neg.d[i].unix_time > unix_time) {
    i++;
    prev_mod_year (&mods);
    year_lut_neg.d[i].year = mods.year;
    year_lut_neg.d[i].is_lyear = is_leap_mod_year (&mods);
    /* To compute the Unix time of THIS year, subtract the length in
       time of the THIS year.  */
    year_lut_neg.d[i].start_dotw = year_lut_neg.d[i-1].start_dotw - 1;
    year_lut_neg.d[i].unix_time =
      year_lut_neg.d[i-1].unix_time - SECS_PER_REG_YEAR;
    if (year_lut_neg.d[i].is_lyear) {
      year_lut_neg.d[i].start_dotw--;
      year_lut_neg.d[i].unix_time -= SECS_PER_DAY;
    }
    if (year_lut_neg.d[i].start_dotw <= 0)
      year_lut_neg.d[i].start_dotw += 7;
    EA_ADD(year_lut_neg);
    { /* Now loop to update the binary lookup table.  */
      long cur_time = (year_lut_neg.d[i-1].unix_time >> 24) - 1;
      long end_time = year_lut_neg.d[i].unix_time >> 24;
      /* Fill in the intermediate entries for this year.  */
      while (cur_time > end_time) {
	EA_INSERT(bin_year_lut_neg, cur_time, i);
	cur_time--;
      }
      /* Fill in the first entry for this year.  */
      EA_INSERT(bin_year_lut_neg, cur_time, i);
    }
  }
}

/* Extend the year lookup table until it can lookup the given Unix
   time.  */
void
extend_year_lut (long unix_time)
{
  if (unix_time >= year_lut_epoch.unix_time)
    extend_year_lut_pos (unix_time);
  else
    extend_year_lut_neg (unix_time);
}

/* Lookup the given year in the year lookup table.  */

/* Convert calendar time to Unix time.  Basically, all the
   considerations mentioned with converting from Unix time to calendar
   time apply, but the final operations are done in reverse.  */

/* ALTERNATIVE.  How to compute the year without a year lookup table.
   Think in terms of cycles.  First figure out which large cycle we
   are on, then progressively figure out where we are in the smaller
   cycles.  Simply divide out the total number of seconds of each
   cycle.  Alas, long division is not so simple, and ostensibly, for
   repeated use, the lookup table would be faster, even on modern
   computers with hardware CPU division instructions.  */

/* TODO: We also need correct handling of timezones and daylight
   saving time.  How to do this?  First ascertain the year.  This is
   needed to determine the particular rules of the given timezone
   locale, because politicians keep changing the laws.  Then for the
   corresponding year, look up the given timezone symbol in the
   timezone database.  */
