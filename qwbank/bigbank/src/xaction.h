/* xaction.h -- Abstract base class for all banking transactions.  */

#ifndef XACTION_H
#define XACTION_H

#include <stdio.h>
#include <time.h>

#include "virtual.h"

/* "Gen" is short for "Generic."  */

struct GenXAction_tag
{
	/* Automatic transaction?  Default false */
	unsigned long automatic : 1;
	/* Transaction ID.  There are effectively two separate
	   "namespaces" for manual and automatic transactions.  It's like
	   the "high bit" is set for automatic transactions.  */
	unsigned long xid : 31;
	/* Absolute date of manual entry, if applicable */
	time_t man_date;
	/* Date -- that is, effective date of transaction */
	time_t date;

	/* Source rule for automatic transaction, if applicable */
	/* Technically I see no real use case for this on second thought,
	   so leave it out for now.  We can always auto-generate
	   later.  */
	/* unsigned long auto_src; */

	/* Future edit status: Current = 0, Outdated = 1 */
	unsigned edit_outdated;
	/* Transaction ID of new record after edit, if applicable */
	unsigned long new_xid;
	/* Transaction ID of old record after edit, if applicable */
	unsigned long old_xid;

	/* Transaction type ID */
	/* Stock meta types: Deleted, Undo */
	unsigned long type_id;
};
typedef struct GenXAction_tag GenXAction;

/* We populate a VTBL with one of these entries for each type ID.  */
struct GenXActionVEntry_tag
{
	/* Size of type-specific data following the generic data
	   structure.  */
	unsigned long sizeof_type;

	/* Standard dumping and parsing methods, required for saving
	   transactions, of course.  Both text and binary are
	   supported.  */
	VIRTUAL unsigned (*text_dump)(unsigned char **buffer, unsigned long *size);
	VIRTUAL unsigned (*text_parse)(FILE *fp);
	VIRTUAL unsigned (*bin_dump)(unsigned char **buffer, unsigned long *size);
	VIRTUAL unsigned (*bin_parse)(FILE *fp);

	/* Apply and undo transaction: Modify the world state by the
	   according command.  No need to create or update transaction
	   objects from executing these methods.  */
	VIRTUAL unsigned (*apply_xaction)(void);
	VIRTUAL unsigned (*undo_xaction)(void);

	/* Check constraints: If history has been edited, this method
	   allows checking whether transaction have become
	   invalidated.  */
	VIRTUAL unsigned (*check_consts)(void);

	/* Get the identity of the actor who performed a transaction.  */
	VIRTUAL unsigned (*audit_get_actor)(unsigned char **buffer, unsigned long *size);
};
typedef struct GenXActionVEntry_tag GenXActionVEntry;

#endif /* not XACTION_H */
