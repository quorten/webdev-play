/* Generate dumbank payment transactions from hospital checkup
   scheduling information.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"
#include "exparray.h"

typedef char* char_ptr;
EA_TYPE(char_ptr);

struct CkupRec_tag
{
  char *date;
  char *time;
  char *doctor;
  char *name;
  char *travel_opt;
};
typedef struct CkupRec_tag CkupRec;

EA_TYPE(CkupRec);

struct WrTransfer_tag
{
  char *date;
  char *time;
  char *pay_from;
  char *pay_to;
  char *amount;
  char *purpose;
};
typedef struct WrTransfer_tag WrTransfer;

void
write_transfer(WrTransfer *xfer, FILE *fout)
{
  fprintf(fout,
	  "\n"
	  "Type:Transfer\n"
	  "File Date:%s %s\n"
	  "Pay From:%s\n"
	  "Pay To:%s\n"
	  "Amount:%s\n"
	  "Purpose:%s\n",
	  xfer->date, xfer->time,
	  xfer->pay_from, xfer->pay_to,
	  xfer->amount, xfer->purpose);
}

/* NOTE: No more than 60 riders may ride at the same base scheduled
   time.  */
void
write_dt_rides(char *base_time, char_ptr_array riders)
{
  unsigned i;
  for (i = 0; i < riders.len; i++) {
    WrTransfer xfer;
    char timebuf[32];
    sprintf(timebuf, "%s:%02d", base_time, i);
    xfer.date = "2020-07-13";
    xfer.time = timebuf;
    xfer.pay_from = riders.d[i];
    xfer.pay_to = "Pom Pom Dusty";
    xfer.amount = "$1.000";
    xfer.purpose = "Drink Truck ride";
    write_transfer(&xfer, stdout);
  }
}

int
main(void)
{
  char_ptr_array lines;
  CkupRec_array records;
  /* Names of Drink Truck arrival riders at 07:30, 09:30, 12:00.  */
  char_ptr_array dt_07;
  char_ptr_array dt_09;
  char_ptr_array dt_12;
  /* Names of Drink Truck departure rider at 17:30 to go back home
     from the capital.  */
  char_ptr_array dt_gohome;

  EA_INIT(lines, 16);
  EA_INIT(records, 16);
  EA_INIT(dt_07, 16);
  EA_INIT(dt_09, 16);
  EA_INIT(dt_12, 16);
  EA_INIT(dt_gohome, 16);

  { /* Skip the first header line.  */
    char temp[256];
    fgets(temp, 256, stdin);
  }

  /* Read in all records.  */
  while (!feof(stdin)) {
    char *cur_pos;
    unsigned i;
    /* TODO IMPROVEMENT: We could use `exp_getline()` here
       instead.  */
    lines.d[lines.len] = xmalloc(sizeof(char) * 256);
    if (fgets(lines.d[lines.len], 256, stdin) == NULL)
      break;
    { /* Clear the newline at the end.  */
      char *str = lines.d[lines.len];
      char *end = &str[strlen(str)-1];
      if (*end == '\n')
	*end = '\0';
    }
    cur_pos = lines.d[lines.len];
    i = 0;
    while (i < 5 && cur_pos != NULL) {
      switch (i) {
      case 0: records.d[records.len].date = cur_pos; break;
      case 1: records.d[records.len].time = cur_pos; break;
      case 2: records.d[records.len].doctor = cur_pos; break;
      case 3: records.d[records.len].name = cur_pos; break;
      case 4: records.d[records.len].travel_opt = cur_pos; break;
      }
      /* Advance to the next field, and insert a null character if
	 necessary.  */
      i++;
      cur_pos = strstr(cur_pos, ": ");
      if (cur_pos != NULL) {
	*cur_pos = '\0';
	cur_pos += 2;
      }
    }
    EA_ADD(lines);
    EA_ADD(records);
  }

  /* The records are already sorted by time.  So, no need to do that
     here.  */

  /* For Drink Truck riders, solve the scheduling problem, determine
     which ride times they take to arrive at the hospital/capital.
     Departing is easy as they all depart at the same time.  */
  {
    unsigned i;
    for (i = 0; i < records.len; i++) {
      if (strcmp(records.d[i].travel_opt, "Drink Truck ride") == 0) {
	if (strcmp(records.d[i].time, "12:00") >= 0) {
	  EA_APPEND(dt_12, records.d[i].name);
	} else if (strcmp(records.d[i].time, "09:30") >= 0) {
	  EA_APPEND(dt_09, records.d[i].name);
	} else { /* (strcmp(records.d[i].time, "07:30") >= 0) */
	  EA_APPEND(dt_07, records.d[i].name);
	}
	EA_APPEND(dt_gohome, records.d[i].name);
      }
    }
  }

  /* Now we can write out all transactions in chronological order,
     keeping working the Drink Truck rides in there in mind.  */
  {
    /* 1: Wrote 07:30 payments.  */
    /* 2: Wrote 09:30 payments.  */
    /* 4: Wrote 12:00 payments.  */
    unsigned char dt_flags = 0;
    unsigned i;
    for (i = 0; i < records.len; i++) {
      WrTransfer xfer;
      char timebuf[32];
      /* TODO FIXME: Do not allow overflow on purpose_buf.  */
      char purpose_buf[128];
      char *secs1 = "00";
      char *secs2 = "01";
      char *amount = "$60.000";
      char *special = "";
      char *doc_official_name = "?";
      /* Write out the Drink Truck rides first if needed, and flag
	 them as done.  */
      if ((dt_flags & 1) == 0 &&
	  strcmp(records.d[i].time, "07:30") >= 0) {
	write_dt_rides("07:30", dt_07);
	dt_flags |= 1;
      }
      if ((dt_flags & 2) == 0 &&
	  strcmp(records.d[i].time, "09:30") >= 0) {
	write_dt_rides("09:30", dt_09);
	dt_flags |= 2;
      }
      if ((dt_flags & 4) == 0 &&
	  strcmp(records.d[i].time, "12:00") >= 0) {
	write_dt_rides("09:30", dt_12);
	dt_flags |= 4;
      }
      /* Write the patient payment, add a few seconds so we don't have
	 multiple payments at the exact same time.  */
      if (strcmp(records.d[i].doctor, "Dr. Doost") == 0)
	{ secs1 = "00"; secs2 = "01";
	  doc_official_name = "Dusty"; }
      else if (strcmp(records.d[i].doctor, "Dr. Dubay") == 0)
	{ secs1 = "02"; secs2 = "03";
	  doc_official_name = "Dubay"; }
      else if (strcmp(records.d[i].doctor, "Dr. Quack") == 0)
	{ secs1 = "04"; secs2 = "05";
	  doc_official_name = "Fling Fling"; }
      sprintf(timebuf, "%s:%s", records.d[i].time, secs1);
      if (strcmp(records.d[i].travel_opt, "Ambulance ride") == 0) {
	amount = "$110.000";
	special = " with ambulance ride";
      } else if (strcmp(records.d[i].travel_opt, "Helicopter ride") == 0) {
	amount = "$160.000";
	special = " with helicopter ride";
      }
      sprintf(purpose_buf, "Regular checkup with %s -- Special Offer%s",
	      records.d[i].doctor, special);

      xfer.date = records.d[i].date;
      xfer.time = timebuf;
      xfer.pay_from = records.d[i].name;
      xfer.pay_to = "Happyou Hospital";
      xfer.amount = amount;
      xfer.purpose = purpose_buf;
      write_transfer(&xfer, stdout);
      /* Write the payment to the corresponding doctor, also adding a
	 few seconds.  */
      sprintf(timebuf, "%s:%s", records.d[i].time, secs2);
      xfer.date = records.d[i].date;
      xfer.time = timebuf;
      xfer.pay_from = "Happyou Hospital";
      xfer.pay_to = doc_official_name;
      xfer.amount = "$5.000";
      xfer.purpose = "Doctor work, regular checkup";
      write_transfer(&xfer, stdout);
    }
    /* Finally, finish up by writing the Drink Truck go home
       payments.  */
    write_dt_rides("17:30", dt_gohome);
  }

  /* Cleanup.  */
  {
    unsigned i;
    for (i = 0; i < lines.len; i++) {
      xfree(lines.d[i]);
    }
  }

  EA_DESTROY(lines);
  EA_DESTROY(records);
  EA_DESTROY(dt_07);
  EA_DESTROY(dt_09);
  EA_DESTROY(dt_12);
  EA_DESTROY(dt_gohome);

  return 0;
}
