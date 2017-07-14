#ifndef __WPBACKUP_DEBUG_H
#define __WPBACKUP_DEBUG_H

#define DEBUG(format, args...) debug_info(format, args)

void debug_info(char *format,...);

#endif /* __WPBACKUP_DEBUG_H */
