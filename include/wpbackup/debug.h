#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG(format, args...) debug_info(format, args)

void debug_info(char *format,...);

#endif /* __DEBUG_H */
