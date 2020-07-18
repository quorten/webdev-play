/* Randomly generate a valid scheduling matrix for Happyou Hospital.
   Three doctors can work in parallel, 15 minute time slots for
   appointments.  */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "exparray.h"

#define MAX_DOCTORS 3

struct TimeSlot_tag
{
  unsigned char time_idx;
  /* Bit set of available doctors at this time.  */
  unsigned char avail_doctors;
};
typedef struct TimeSlot_tag TimeSlot;

EA_TYPE(TimeSlot);

unsigned char
popcount(unsigned char bit_set)
{
  unsigned char result = 0;
  unsigned char i;
  for (i = 0; i < 8; i++) {
    result += (bit_set >> i) & 1;
  }
  return result;
}

unsigned char
get_bit_set_pick(unsigned char bit_set, unsigned char pick_eff_idx)
{
  unsigned char result = 0;
  unsigned char i;
  unsigned char eff_idx = 0; /* Effective index */
  for (i = 0; i < 8; i++) {
    if (((bit_set >> i) & 1)) {
      if (eff_idx == pick_eff_idx)
	return 1 << i;
      eff_idx++;
    }
  }
  return 0;
}

void
print_pick (TimeSlot *pick, FILE *fout)
{
  const char *date = "2020-07-13";
  unsigned char hour = pick->time_idx / 4 + 8;
  unsigned char minute = (pick->time_idx % 4) * 15;
  const char *doctor = "Unknown?";
  const char *ride = "Walk/other";
  if (hour >= 12) /* Skip 1-hour lunch break */
    hour++;
  switch (pick->avail_doctors) {
  case 1:
    doctor = "Dr. Doost";
    break;
  case 2:
    doctor = "Dr. Quack";
    break;
  case 4:
    doctor = "Dr. Dubay";
    break;
  }

  /* While we're at it, we might as well generate picks for travel
     options.

     The statistics:

     * 70% walkers
     * 20% Drink Truck riders
     * 7% Ambulance riders
     * 3% Helicopter riders
  */
  {
    unsigned char ride_rand = (unsigned char)(rand() % 100 + 1);
    if (ride_rand >= 96)
      ride = "Helicopter ride";
    else if (ride_rand >= 90)
      ride = "Ambulance ride";
    else if (ride_rand >= 70)
      ride = "Drink Truck ride";
    /* else ride = "Walk/other"; */
  }

  printf("%s: %02d:%02d: %s: %s\n", date, hour, minute, doctor, ride);
}

int
main(int argc, char* argv[])
{
  TimeSlot_array time_slots;
  unsigned char i;
  time_t seed;
  EA_INIT(time_slots, 4 * 8 + 1);

  /* Initialize array of available time slots.  */
  for (i = 0; i < 4 * 8; i++) {
    time_slots.d[i].time_idx = i;
    time_slots.d[i].avail_doctors = (1 << MAX_DOCTORS) - 1;
    EA_ADD(time_slots);
  }

  /* Initialize random seed.  */
  if (argc == 2)
    seed = atoi(argv[1]);
  else
    seed = time(NULL);
  fprintf(stderr, "Random seed = %ld\n", seed);
  srand(seed);

  /* Show the table header for posterity.  */
  puts("Date: Time: Doctor: Travel option");

  while (time_slots.len > 0) {
    /* Now randomly pick a time slot, randomly pick a doctor, and
       remove that doctor from the set of available doctors.  If there
       are no more doctors left, then remove the time slot from the
       array.  */
    TimeSlot cur_pick;
    unsigned char avail_docs_len;
    unsigned char bit_set_pick;
    i = (unsigned char)(rand() % time_slots.len);
    cur_pick.time_idx = time_slots.d[i].time_idx;
    avail_docs_len = popcount(time_slots.d[i].avail_doctors);
    bit_set_pick = (unsigned char)(rand() % avail_docs_len);
    cur_pick.avail_doctors =
      get_bit_set_pick(time_slots.d[i].avail_doctors, bit_set_pick);
    time_slots.d[i].avail_doctors &= ~cur_pick.avail_doctors;
    avail_docs_len--;
    if (avail_docs_len == 0) {
      EA_REMOVE_FAST(time_slots, i);
    }
    /* Print out the result of the pick immediately.  */
    print_pick(&cur_pick, stdout);
  }

  EA_DESTROY(time_slots);
  return 0;
}
