#ifndef CUSTOM_SCHED_VALIDATOR
#define CUSTOM_SCHED_VALIDATOR

#include <isl/schedule.h>
#include <isl/set.h>
#include <isl/union_map.h>
#include <isl/union_set.h>

int validate_custom_schedule(isl_union_map *deps, isl_schedule *schedule);

#endif
