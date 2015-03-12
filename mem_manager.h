#include "linuxenv.h"

//mem_manager.c provide:
int32 newCONS(int32 x, int32 y);
void gc(void);
void gcmark(int32 p);

void traceprint(int32 v, int16 osw);
