#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern uint64_t handler_labels[];
extern uint64_t *bootstrap_stack_top;

#define GLOBAL_STACK_BOTTOM bootstrap_stack_top

#endif /* __GENERAL_H__ */
