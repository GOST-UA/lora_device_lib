#ifndef LDL_H
#define LDL_H

#include <stdbool.h>

void ldl_init(void);
void ldl_tick(void **state, void (*on_ready)(void **));
bool ldl_join(void);

#endif
