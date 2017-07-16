#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __DEBUG
	#define DEBUG(format, args...) debug_info(format, args)
#else
	#define DEBUG(format, args...)
#endif

void debug_info(char *format,...);

#endif /* __DEBUG_H */
