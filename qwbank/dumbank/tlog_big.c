/* Functions to read and write transaction log, then show account
   totals.  */

/* Use 64-bit time so that we can handle dates after the year 2038.
   Unfortunately, this has no effect on 32-bit Linux with glibc due to
   the attitude of the developers.  */
#define __TIMESIZE 64

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include <gmp.h>

#include "xmalloc.h"
#include "exparray.h"

#define MODE_KEY 0
#define MODE_VALUE 1

#define TYPE_UNDEF 0
#define TYPE_ACCOUNT 1
#define TYPE_TRANSFER 2
#define TYPE_AUTO_PAY_RULE 3
#define TYPE_UPD_AUTO_PAY 4

/* MAX_ACCOUNTS is one greater than the allowed accounts.
   (x < MAX_ACCOUNTS)  */
#define MAX_ACCOUNTS ((unsigned short)-1)
#define INVALID_ACCOUNT ((unsigned short)-1)

typedef enum { false, true } bool;

typedef char* char_ptr;
EA_TYPE(char);
EA_TYPE(char_ptr);

struct Account_tag
{
  time_t creation_date;
  char *name;
  char *acct_type;
  /* `long long' supports 18 full decimal digits.  12-figure values,
     we're okay, but 15-figure values are pushing to the limit.  Also,
     we only add and subtract for money transactions, so multiply
     overflow is not a concern.  */
  mpz_t balance; /* Balance in "mills", 1/1000 of a dollar.  */
};
typedef struct Account_tag Account;
EA_TYPE(Account);

/* N.B. We do allow a transfer of negative money, but that is really
   the same as swapping "pay from" and "pay to".  */
struct Transfer_tag
{
  time_t file_date;
  unsigned short pay_from;
  unsigned short pay_to;
  mpz_t amount;
  char *purpose;
};
typedef struct Transfer_tag Transfer;

struct AutoPayRule_tag
{
  /* If `autopay_monthly' is `false', pay on constant time
     interval.  */
  unsigned char autopay_monthly;
  time_t payday1;
  time_t interval;
  time_t last_payday;
  /* Now we just fill in fields for a Transfer record.  */
  unsigned short pay_from;
  unsigned short pay_to;
  mpz_t amount;
  char *purpose;
};
typedef struct AutoPayRule_tag AutoPayRule;
EA_TYPE(AutoPayRule);

unsigned char field_parse_type = TYPE_UNDEF;
union {
  Account account;
  Transfer transfer;
  AutoPayRule rule;
} field_parse_record;

time_t report_time;
struct tm report_cal_time;
Account_array accounts;
AutoPayRule_array auto_pay_rules;

bool parse_key_value (void);
bool begin_process_fields (void);
bool process_field(char *key, char *value);
bool end_process_fields (void);
unsigned short search_account_id (char *name);
void execute_transfer (Transfer *transfer);
bool update_autopay_rules (void);
bool update_monthly_auto_pay (AutoPayRule *rule,
			      struct tm *start, struct tm *end);
bool update_interval_auto_pay (AutoPayRule *rule,
			       time_t start_time, time_t end_time);
void print_account_totals (void);

int
main (int argc, char *argv[])
{
  char *query_string;
  struct flock lock_info;
  lock_info.l_type = F_RDLCK;
  lock_info.l_whence = SEEK_SET;
  lock_info.l_start = 0;
  lock_info.l_len = 0;
  lock_info.l_pid = 0;
  int fd = open("dumbank.log", O_RDONLY, 0);
  if (fd == -1) {
    fprintf (stderr, "Error opening log.\n");
    return 1;
  }
  if (fcntl (fd, F_SETLK, &lock_info) == -1) {
    fprintf (stderr, "Error locking log.\n");
    return 1;
  }
  dup2 (fd, STDIN_FILENO);

  query_string = getenv ("QUERY_STRING");
  memset (&report_cal_time, 0, sizeof(struct tm));

  if (argc == 2) {
    strptime (argv[1], "%Y-%m-%d %H:%M:%S", &report_cal_time);
    report_cal_time.tm_isdst = -1;
    report_time = mktime (&report_cal_time);
  } else if (query_string && strncmp (query_string, "date=", 5) == 0) {
    strptime (&query_string[5], "%Y-%m-%d%%20%H:%M:%S", &report_cal_time);
    report_cal_time.tm_isdst = -1;
    report_time = mktime (&report_cal_time);
  } else {
    report_time = time (NULL);
    localtime_r (&report_time, &report_cal_time);
  }

  if (!parse_key_value ()) {
    fprintf (stderr, "Error parsing input.\n");
    close (fd);
    return 1;
  }
  close (fd);
  update_autopay_rules ();
  print_account_totals ();
  return 0;
}

bool
parse_key_value (void)
{
  bool retval = true;
  int c;
  unsigned char parse_mode = MODE_KEY;
  char *last_key = NULL;
  char_array key;
  char_array value;

  if (!begin_process_fields ())
    return false;

  EA_INIT(key, 16);
  EA_INIT(value, 16);

  while ((c = getchar ()) != EOF) {
    if (parse_mode == MODE_KEY && c == ':') {
      EA_APPEND(key, '\0');
      parse_mode = MODE_VALUE;
      if ((last_key != NULL && strcmp (last_key, key.d) == 0) ||
	  (last_key != NULL && key.d[0] == '\0')) {
	/* Convert the null character to a newline on the current
	   value, and keep appending.  */
	value.d[value.len-1] = '\n';
      } else {
	if (last_key != NULL) {
	  /* Process the old field, now that we know we're totally
	     done reading it.  */
	  if (!process_field (last_key, value.d))
	    { retval = false; break; }
	  /* Clear the old data.  */
	  xfree (last_key);
	  value.len = 0;
	}
	/* Save the new last key.  */
	last_key = xstrdup (key.d);
      }
      continue;
    } else if (c == '\n') {
      if (parse_mode == MODE_VALUE) {
	EA_APPEND(value, '\0');
	parse_mode = MODE_KEY;
	key.len = 0;
      } else { /* (parse_mode == MODE_KEY) */
	/* Do nothing, really.  If we've read an incomplete key, clear
	   it.  */
	key.len = 0;
      }
      continue;
    }
    else if (parse_mode == MODE_KEY)
      EA_APPEND(key, (char)c);
    else /* (parse_mode == MODE_VALUE) */
      EA_APPEND(value, (char)c);
  }
  if (last_key != NULL && value.len > 0) {
    if (value.d[value.len-1] != '\0')
      EA_APPEND(value, '\0');
    /* Process the final field.  */
    retval = process_field (last_key, value.d);
  }
  /* Indicate this is the end of the data to the next level.  */
  retval &= end_process_fields ();
  /* Clear the parser data.  */
  key.len = 0;
  value.len = 0;
  if (last_key != NULL)
    xfree (last_key);
  last_key = NULL;
  parse_mode = MODE_KEY;

  /* Cleanup.  */
  EA_DESTROY(key);
  EA_DESTROY(value);

  return retval;
}

bool
begin_process_fields (void)
{
  EA_INIT(accounts, 16);
  EA_INIT(auto_pay_rules, 16);

  field_parse_type = TYPE_UNDEF;
  return true;
}

bool
process_field (char *key, char *value)
{
  /* fprintf (stderr, "%s:%s\n", key, value); */

  if (strcmp (key, "Type") == 0) {
    if (field_parse_type != TYPE_UNDEF) {
      /* Commit the previously read record.  */
      if (!end_process_fields ())
	return false;
    }
    if (strcmp (value, "Account") == 0) {
      field_parse_type = TYPE_ACCOUNT;
    } else if (strcmp (value, "Transfer") == 0) {
      field_parse_type = TYPE_TRANSFER;
    } else if (strcmp (value, "Auto-Pay Rule") == 0) {
      field_parse_type = TYPE_AUTO_PAY_RULE;
    }
    return true;
  }

  if (field_parse_type == TYPE_ACCOUNT) {
    if (strcmp (key, "Name") == 0) {
      field_parse_record.account.name = xstrdup (value);
      /* Also initialize the balance to zero since we don't have a
	 field to set it explicitly.  */
      mpz_init (field_parse_record.account.balance);
    } else if (strcmp (key, "Account Type") == 0) {
      field_parse_record.account.acct_type = xstrdup (value);
    } else if (strcmp (key, "Creation Date") == 0) {
      struct tm cal_time;
      memset (&cal_time, 0, sizeof(struct tm));
      strptime (value, "%Y-%m-%d %H:%M:%S", &cal_time);
      cal_time.tm_isdst = -1;
      field_parse_record.account.creation_date = mktime (&cal_time);
    } else
      return false;
  } else if (field_parse_type == TYPE_TRANSFER) {
    if (strcmp (key, "File Date") == 0) {
      struct tm cal_time;
      memset (&cal_time, 0, sizeof(struct tm));
      strptime (value, "%Y-%m-%d %H:%M:%S", &cal_time);
      cal_time.tm_isdst = -1;
      field_parse_record.transfer.file_date = mktime (&cal_time);
    } else if (strcmp (key, "Pay From") == 0) {
      field_parse_record.transfer.pay_from = search_account_id (value);
      if (field_parse_record.transfer.pay_from == INVALID_ACCOUNT)
	return false;
    } else if (strcmp (key, "Pay To") == 0) {
      field_parse_record.transfer.pay_to = search_account_id (value);
      if (field_parse_record.transfer.pay_to == INVALID_ACCOUNT)
	return false;
    } else if (strcmp (key, "Amount") == 0) {
      char *decimal = strchr (value, '.');
      /* long long dollars = 0; */
      int mills = 0;
      if (decimal != NULL) {
	*decimal = '\0';
	sscanf (decimal + 1, "%03d", &mills);
      }
      mpz_init_set_str (field_parse_record.transfer.amount, value + 1, 10);
      mpz_mul_ui (field_parse_record.transfer.amount,
		  field_parse_record.transfer.amount, 1000);
      if (mpz_sgn (field_parse_record.transfer.amount) < 0 && mills > 0)
	mills = -mills; /* Take care of proper signs.  */
      if (mills >= 0)
	mpz_add_ui (field_parse_record.transfer.amount,
		    field_parse_record.transfer.amount, mills);
      else
	mpz_sub_ui (field_parse_record.transfer.amount,
		    field_parse_record.transfer.amount, -mills);
    } else if (strcmp (key, "Purpose") == 0) {
      field_parse_record.transfer.purpose = xstrdup (value);
    } else
      return false;
  } else if (field_parse_type == TYPE_AUTO_PAY_RULE) {
    if (strcmp (key, "Auto-Pay Monthly") == 0) {
      if (strcmp (value, "Yes") == 0)
	field_parse_record.rule.autopay_monthly = true;
      else
	field_parse_record.rule.autopay_monthly = false;
    } else if (strcmp (key, "Pay-Day #1") == 0) {
      /* TODO FIXME: Factor the following copy & pasted code.  */
      struct tm cal_time;
      memset (&cal_time, 0, sizeof(struct tm));
      strptime (value, "%Y-%m-%d %H:%M:%S", &cal_time);
      cal_time.tm_isdst = -1;
      field_parse_record.rule.payday1 = mktime (&cal_time);
      field_parse_record.rule.last_payday = (time_t)0;
    } else if (strcmp (key, "Interval") == 0) {
      if (strcmp (value, "Daily") == 0) {
	field_parse_record.rule.interval = 1 * 24 * 60 * 60;
      } else if (strcmp (value, "Weekly") == 0) {
	field_parse_record.rule.interval = 7 * 24 * 60 * 60;
      } else if (strcmp (value, "Bi-Weekly") == 0) {
	field_parse_record.rule.interval = 14 * 24 * 60 * 60;
      } else if (strcmp (value, "Monthly") == 0) {
	/* N.B. We could also set interval to zero to indicate that
	   the field is unused, but we set it to 4 weeks for
	   posterity.  */
	field_parse_record.rule.interval = 28 * 24 * 60 * 60;
      }
      /* TODO FIXME: Factor the following copy & pasted code.  */
    } else if (strcmp (key, "Pay From") == 0) {
      field_parse_record.rule.pay_from = search_account_id (value);
      if (field_parse_record.rule.pay_from == INVALID_ACCOUNT)
	return false;
    } else if (strcmp (key, "Pay To") == 0) {
      field_parse_record.rule.pay_to = search_account_id (value);
      if (field_parse_record.rule.pay_to == INVALID_ACCOUNT)
	return false;
    } else if (strcmp (key, "Amount") == 0) {
      char *decimal = strchr (value, '.');
      /* long long dollars = 0; */
      int mills = 0;
      if (decimal != NULL) {
	*decimal = '\0';
	sscanf (decimal + 1, "%03d", &mills);
      }
      mpz_init_set_str (field_parse_record.rule.amount, value + 1, 10);
      mpz_mul_ui (field_parse_record.rule.amount,
		  field_parse_record.rule.amount, 1000);
      if (mpz_sgn (field_parse_record.rule.amount) < 0 && mills > 0)
	mills = -mills; /* Take care of proper signs.  */
      if (mills >= 0)
	mpz_add_ui (field_parse_record.rule.amount,
		    field_parse_record.rule.amount, mills);
      else
	mpz_sub_ui (field_parse_record.rule.amount,
		    field_parse_record.rule.amount, -mills);
    } else if (strcmp (key, "Purpose") == 0) {
      field_parse_record.rule.purpose = xstrdup (value);
    } else
      return false;
  }

  return true;
}

bool
end_process_fields (void)
{
  if (field_parse_type == TYPE_UNDEF)
    ; /* Do nothing.  */
  else if (field_parse_type == TYPE_ACCOUNT) {
    EA_APPEND_MULT(accounts, &field_parse_record, 1);
  } else if (field_parse_type == TYPE_TRANSFER) {
    /* Execute the transfer immediately.  */
    execute_transfer (&field_parse_record.transfer);
    /* Now destroy the transfer object since we no longer need it.  */
    mpz_clear (field_parse_record.transfer.amount);
    xfree (field_parse_record.transfer.purpose);
  } else if (field_parse_type == TYPE_AUTO_PAY_RULE) {
    EA_APPEND_MULT(auto_pay_rules, &field_parse_record, 1);
  } else
    return false;
  return true;
}

unsigned short
search_account_id (char *name)
{
  unsigned short i;
  /* TODO FIXME: Enforce proper limits on length of accounts.  */
  for (i = 0; i < accounts.len; i++) {
    if (strcmp (accounts.d[i].name, name) == 0)
      return i;
  }
  return INVALID_ACCOUNT;
}

void
execute_transfer (Transfer *transfer)
{
  unsigned short from = transfer->pay_from;
  unsigned short to = transfer->pay_to;
  mpz_t *amount = &transfer->amount;
  if (transfer->file_date > report_time)
    return; /* Do not execute future transfers.  */
  mpz_sub (accounts.d[from].balance,
	   accounts.d[from].balance, *amount);
  mpz_add (accounts.d[to].balance,
	   accounts.d[to].balance, *amount);
}

bool
update_autopay_rules (void)
{
  unsigned i;

  for (i = 0; i < auto_pay_rules.len; i++) {
    if (auto_pay_rules.d[i].autopay_monthly) {
      struct tm cal_payday1;
      memset (&cal_payday1, 0, sizeof(struct tm));
      localtime_r (&auto_pay_rules.d[i].payday1, &cal_payday1);
      update_monthly_auto_pay (&auto_pay_rules.d[i],
			       &cal_payday1,
			       &report_cal_time);
    } else {
      update_interval_auto_pay (&auto_pay_rules.d[i],
				auto_pay_rules.d[i].payday1,
				report_time);
    }
    xfree (auto_pay_rules.d[i].purpose);
  }

  EA_DESTROY(auto_pay_rules);
  return true;
}

/* Generate monthly auto-pay transactions for the given time interval,
   inclusive.  Payment is given on the first day of each month.  */
/* TODO: We also need to pass through the data structure needed to
   complete the auto-pay transaction.  */
bool
update_monthly_auto_pay (AutoPayRule *rule,
			 struct tm *start, struct tm *end)
{
  struct tm cal_payday1;
  unsigned mon_payday = 1; /* pay day in the month */
  Transfer transfer;

  localtime_r (&rule->payday1, &cal_payday1);

  transfer.pay_from = rule->pay_from;
  transfer.pay_to = rule->pay_to;
  mpz_init_set (transfer.amount, rule->amount);
  transfer.purpose = rule->purpose;

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
      struct tm cal_time;
      if (i == year1 && j == month1 && mon_payday < day1)
	continue;
      /* Execute transaction creation API here.  */
      memset (&cal_time, 0, sizeof(struct tm));
      cal_time.tm_year = i;
      cal_time.tm_mon = j;
      cal_time.tm_mday = mon_payday;
      cal_time.tm_hour = cal_payday1.tm_hour;
      cal_time.tm_min = cal_payday1.tm_min;
      cal_time.tm_sec = cal_payday1.tm_sec;
      cal_time.tm_isdst = -1;
      transfer.file_date = mktime (&cal_time);
      execute_transfer (&transfer);
    }
  }

  mpz_clear (transfer.amount);
  mpz_clear (rule->amount);

  return true;
}

/* Generate constant time interval auto-pay transactions for the given
   time interval, inclusive, using the given pay-day #1 to determine
   what day of the week to use.  Payment is given on the first day of
   each month.  */
/* TODO: We also need to pass through the data structure needed to
   complete the auto-pay transaction.  */
bool
update_interval_auto_pay (AutoPayRule *rule,
			  time_t start_time, time_t end_time)
{
  time_t pay_interval = rule->interval;
  time_t mod_interval;
  time_t cur_time;
  struct tm cal_time;
  Transfer transfer;
  transfer.pay_from = rule->pay_from;
  transfer.pay_to = rule->pay_to;
  mpz_init_set (transfer.amount, rule->amount);
  transfer.purpose = rule->purpose;

  if (start_time < rule->payday1)
    start_time = rule->payday1;
  if (rule->last_payday && end_time > rule->last_payday)
    end_time = rule->last_payday;

  mod_interval = (start_time - rule->payday1) % pay_interval;
  cur_time = start_time;

  if (mod_interval != 0)
    cur_time += pay_interval - mod_interval;

  while (cur_time <= end_time) {
    /* Execute transaction creation API here.  */
    transfer.file_date = cur_time;
    execute_transfer (&transfer);
    cur_time += pay_interval;
  }

  mpz_clear (transfer.amount);
  mpz_clear (rule->amount);

  return true;
}

void
print_account_totals (void)
{
  char time_buf[128];
  unsigned i;
  fputs ("Content-Type: text/plain\n\n", stdout);
  strftime (time_buf, 128, "%Y-%m-%d %H:%M:%S", &report_cal_time);
  printf ("Account totals as of %s:\n", time_buf);
  for (i = 0; i < accounts.len; i++) {
    mpz_t dollars;
    mpz_t mills;
    long mills_disp;
    mpz_init (dollars);
    mpz_init (mills);
    mpz_tdiv_qr_ui (dollars, mills, accounts.d[i].balance, 1000);
    /* Do not display negative mills, as we already have the negative
       sign on dollars.  */
    if (mpz_sgn (mills) < 0) mpz_neg (mills, mills);
    /* Do not display future accounts not yet created.  */
    if (accounts.d[i].creation_date <= report_time) {
      mills_disp = mpz_get_si (mills);
      /* dollars_str = mpz_get_str (NULL, 10, balance);
      mills_str = mpz_get_str (NULL, 10, mills);
      free (dollars_str);
      free (mills_str); */
      printf ("%s: $", accounts.d[i].name);
      mpz_out_str (stdout, 10, dollars);
      printf(".%03d\n", mills_disp);
      mpz_clear (dollars);
      mpz_clear (mills);
      mpz_clear (accounts.d[i].balance);
    }
    xfree (accounts.d[i].name);
    xfree (accounts.d[i].acct_type);
  }
  EA_DESTROY(accounts);
}
