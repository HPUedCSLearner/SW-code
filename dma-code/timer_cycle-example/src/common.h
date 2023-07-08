
#include <athread.h>
#include <stdlib.h>

#define cpeNum (64)
#define defaultSize (1 * 1024 * 1024) // 1M

typedef struct Args {
    double data[cpeNum][defaultSize];
}Args;


unsigned long slave_timer();
unsigned long master_timer();