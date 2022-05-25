#include <stdio.h>
#include <stdlib.h>
#include "libgcov.h"

static unsigned long total_count = 0;
unsigned int dry_run_total_count = 0;
static struct {
	struct {
		const char *fname;
		unsigned int count;
	} items[1024];
	unsigned int index;
} collection;

/*
 * Inspired from gcov_clear().
 */

static void gcov_read(const struct gcov_info *list)
{
	const struct gcov_info *gi_ptr;

	/* Use gi_ptr to walk object files in list. */
	for (gi_ptr = list; gi_ptr; gi_ptr = gi_ptr->next) {
		unsigned f_ix;
		//printf("object file output: %s\n", gi_ptr->filename);

		/* Use f_ix and gfi_ptr to walk functions in gi_ptr. */
		for (f_ix = 0; f_ix < gi_ptr->n_functions; f_ix++) {
			unsigned t_ix;
			const struct gcov_fn_info *gfi_ptr = gi_ptr->functions[f_ix];

			if (!gfi_ptr || gfi_ptr->key != gi_ptr)
				continue;

			//printf("function ident: 0x%016x (%u)\n", gfi_ptr->ident, gfi_ptr->ident);
			/* Use t_ix ci_ptr to walk basic blocs in gfi_ptr. */
			const struct gcov_ctr_info *ci_ptr = gfi_ptr->ctrs;
			for (t_ix = 0; t_ix != GCOV_COUNTERS; t_ix++) {
				size_t i;
				if (!gi_ptr->merge[t_ix])
					continue;

				//printf("ci_ptr: %p\n", ci_ptr);
				for (i = 0; i < ci_ptr->num; i++)
					if (ci_ptr->values[i] != 0) {
						//printf("object file output: %s, value: %u\n", gi_ptr->filename, ci_ptr->values[i]);
						collection.items[collection.index].fname = gi_ptr->filename;
						collection.items[collection.index].count = ci_ptr->values[i];
						collection.index += 1;
						total_count += ci_ptr->values[i];
					}
				memset(ci_ptr->values, 0, sizeof (gcov_type) * ci_ptr->num);
				ci_ptr++;
			}
		}
	}
}

/*
 * Inspired from __gcov_reset_int().
 */

static void gcov_read_ctrs(void)
{
	struct gcov_root *root;

	/*
	 * If we're compatible with the master, iterate over everything, otherwise just do us.
	 */
	for (root = __gcov_master.version == GCOV_VERSION
			? __gcov_master.root : &__gcov_root; root; root = root->next)
		gcov_read(root->list);
}

static void do_xor(unsigned int v[], unsigned int len, unsigned int key)
{
	size_t i;

	for (i = 0; i < len; i++)
		v[i] = v[i] ^ key;
}

static void do_avg(unsigned int v[], unsigned int len)
{
	size_t i;

	for (i = 0; i < len-1; i++)
		v[i+1] = (v[i] ^ v[i+1]) / 2;
}

static void swap(unsigned int *a, unsigned int *b)
{
	unsigned int aux;

	aux = *a;
	*a = *b;
	*b = aux;
}

static void do_sort(unsigned int v[], unsigned int len)
{
	unsigned char sorted = 0;
	size_t i;

	while (sorted == 0) {
		sorted = 1;
		for (i = 0; i < len-1; i++)
			if (v[i] > v[i+1]) {
				swap(&v[i], &v[i+1]);
				sorted = 0;
			}
	}
}

static inline void begin_coverage(void)
{
	total_count = 0;
	__gcov_reset();
}

static inline void end_coverage(void)
{
	gcov_read_ctrs();
	total_count -= dry_run_total_count;
	printf("total count: %lu\n", total_count);
}

#define NUM_ITEMS	1024

int main(void)
{
	unsigned int *v;

	/*
	 * Collect coverage for dry run, i.e. no actual function is called.
	 * There is "recursive" coverage, due to __gcov_reset() calling memset().
	 */
	begin_coverage();
	end_coverage();
	{
		unsigned int i;
		printf("* dry run coverage:\n");
		for (i = 0; i < collection.index; i++)
			printf("%s [%u]\n", collection.items[i].fname, collection.items[i].count);
	}
	dry_run_total_count = total_count;

	/* Collect coverage for printf() call. */
	begin_coverage();
	printf("Hello world!\n");
	end_coverage();

	/* Collect coverage for malloc() call. */
	begin_coverage();
	v = malloc(NUM_ITEMS * sizeof(*v));
	if (v == NULL) {
		fprintf(stderr, "Unable to allocate memory.\n");
		return -1;
	}
	end_coverage();

	/*
	 * Collect coverage for local function calls.
	 * This will end up being 0 if coverage is not enabled for this
	 * file.
	 */
	begin_coverage();
	do_xor(v, NUM_ITEMS, 0xabcdef01);
	do_avg(v, NUM_ITEMS);
	do_sort(v, NUM_ITEMS);
	end_coverage();

	return 0;
}
