/* gidalloc.h -- Global ID allocation variables.  That is, ID
   allocation variables that transaction-based undo cannot be
   supported on, since these are used for ID variables that persist
   beyond the undo layer.  */

#ifndef GIDALLOC_H
#define GIDALLOC_H

extern unsigned long g_next_xid;
extern unsigned long g_next_accnt_id;

#endif /* not GIDALLOC_H */
