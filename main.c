#include <stdio.h>
#include <stdlib.h>

/* Comment this out to disable custom coverage. */
#define CUSTOM_GCOV	1

#if CONFIG_OPTIMIZE_PGO_GENERATE
void set_path_gcov_files(void);
#if CUSTOM_GCOV
void __gcov_reset(void);
void __gcov_dump(void);
#endif
#endif

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

#define NUM_ITEMS	1024

int main(void)
{
	unsigned int *v;

#if CONFIG_OPTIMIZE_PGO_GENERATE
	set_path_gcov_files();
#endif

	printf("Hello world!\n");

	v = malloc(NUM_ITEMS * sizeof(*v));
	if (v == NULL) {
		fprintf(stderr, "Unable to allocate memory.\n");
		return -1;
	}

	do_xor(v, NUM_ITEMS, 0xabcdef01);
	do_avg(v, NUM_ITEMS);

#if CONFIG_OPTIMIZE_PGO_GENERATE && CUSTOM_GCOV
	__gcov_reset();
#endif

	do_sort(v, NUM_ITEMS);

#if CONFIG_OPTIMIZE_PGO_GENERATE && CUSTOM_GCOV
	__gcov_dump();
#endif

	return 0;
}
