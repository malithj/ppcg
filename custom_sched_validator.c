#include "custom_sched_validator.h"

struct itr_data {
	int *arr;
	int idx;
};

/* This function is called for each set in a union_set.
 * If the name of the set matches data->type, we store the
 * set in data->res.
 */
static isl_stat validate_dependency(__isl_take isl_set *size, void *user)
{
	const char *name;
	struct itr_data *d = (struct itr_data *)user;

	name = isl_set_get_tuple_name(size);
	int n = isl_set_dim(size, isl_dim_set);
	int met_zero = 0;
	int met_positive = 0;

	for (int i = 0; i < n; ++i) {
		isl_val *v = isl_set_plain_get_val_if_fixed(size, isl_dim_out, i);
		if (isl_val_is_neg(isl_val_copy(v))) {
			if (met_positive) {
				d->arr[d->idx] = 1;
				d->idx += 1;
				return isl_stat_ok;
			} else {
				d->arr[d->idx] = 0;
				d->idx += 1;
				return isl_stat_ok;
			}
		} else if (isl_val_is_pos(isl_val_copy(v))) {
			met_positive = 1;
		} else if (isl_val_is_zero(isl_val_copy(v))) {
			met_zero = 1;
		}
		isl_val_free(v);
	}
	isl_set_free(size);
	if (!met_positive && !met_zero) {
		d->arr[d->idx] = 1;
		d->idx += 1;
	}
	d->arr[d->idx] = 1;
	d->idx += 1;
	return isl_stat_ok;
}

int validate_custom_schedule(isl_union_map *deps, isl_schedule *custom_isl_schedule)
{
	isl_union_map *custom_schedule = isl_schedule_get_map(custom_isl_schedule);
	isl_union_map *deps_ = isl_union_map_apply_range(
	    isl_union_map_copy(deps), isl_union_map_copy(custom_schedule));
	isl_union_map *result = isl_union_map_apply_domain(deps_, custom_schedule);
	isl_union_set *new_deltas = isl_union_map_deltas(result);

	struct itr_data data;
	int sz = isl_union_set_n_set(isl_union_set_copy(new_deltas));
	data.arr = (int *)malloc(sizeof(int) * sz);
	data.idx = 0;
	isl_union_set_foreach_set(new_deltas, &validate_dependency, &data);
	// isl_union_set_dump(new_deltas);
	isl_union_set_free(new_deltas);
	int state = 1;
	int i;
	for (i = 0; i < sz; ++i) {
		if (!data.arr[i]) {
			state = 0;
			break;
		}
	}

	if (state) {
		printf("dependencies satisfied\n");
		return 1;
	} else {
		printf("dependencies violated\n");
		return 0;
	}
	free(data.arr);
}