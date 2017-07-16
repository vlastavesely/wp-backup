#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __DEBUG
	#define DEBUG(format...) debug_info(format)
#else
	#define DEBUG(format...)
#endif

void debug_info(char *format,...);

#endif /* __DEBUG_H */
