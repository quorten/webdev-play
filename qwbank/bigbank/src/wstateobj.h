/* wstateobj.h -- Abstract base class for all banking world state
   objects.  */

#ifndef WSTATEOBJ_H
#define WSTATEOBJ_H

#include "virtual.h"

/* "Gen" is short for "Generic."  */

struct GenWStateObj_tag
{
	unsigned long auto_rule : 1; /* Is this an automation rule?  */
	unsigned long type_id : 31;
	unsigned long obj_id; /* Object ids are namespaced by type ids */
	/* N.B. Technically we don't need to include this in the base
	   class, we could create a subclass, but we do this for now due
	   to default structure padding and whatnot.  */
	/* If an automation rule, this is the date of the last automation
	   update.  */
	time_t last_date;
};
typedef struct GenWStateObj_tag GenWStateObj;

/* This is our more specific abstract type for automation rules.  As
   previously discussed, just do a typedef for now.  */
typedef GenWStateObj GenAutoRuleObj;

/* We populate a VTBL with one of these entries for each type ID.  */
struct GenWStateObjVEntry_tag
{
  /* Standard dumping and parsing methods, required for saving
     transactions, of course.  Both text and binary are supported.  */
  VIRTUAL unsigned (*text_dump)(unsigned char **buffer, unsigned long *size);
  VIRTUAL unsigned (*text_parse)(FILE *fp);
  VIRTUAL unsigned (*bin_dump)(unsigned char **buffer, unsigned long *size);
  VIRTUAL unsigned (*bin_parse)(FILE *fp);
};
typedef struct GenWStateObjVEntry_tag GenWStateObjVEntry;

struct GenAutoRuleObjVEntry_tag
{
	/* Execute an automation rule to update it from its last date to
	   the new date.  If `new_date' is older than `last_date', then
	   automation rules are undone.  */
	VIRTUAL unsigned (*update_auto_rule)(time_t new_date);
};
typedef struct GenAutoRuleObjVEntry_tag GenAutoRuleObjVEntry;

#endif /* not WSTATEOBJ_H */
