/*
 * Implementation of a double-hashed, open-addressed hash table in C.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hash_table.h"
#include "prime.h"

/*----------------------------------------------------------------------------*/

/* Initialize ht_item(s) */
static ht_item* ht_new_item(const char* k, const char* v)
{
	ht_item* it = malloc(sizeof(ht_item));

	it->key = strdup(k);
	it->val = strdup(v);
	return it;
}

/* Pre-allocate the table in memory */
static ht_hash_table* ht_new_sized(const int base_size)
{
	ht_hash_table* ht = malloc(sizeof(ht_hash_table));
	ht->base_size = base_size;

	ht->size=next_prime(ht->base_size);

	ht->cntr = 0;
	ht->items = calloc((size_t)ht->size, sizeof(ht_item*));
	return ht;
}

/* Initialize a hash table */
ht_hash_table* ht_new_table(void)
{
	return ht_new_sized(HT_INIT_BASE_SIZE);
}

/* Delete an item from the hash table */
static void ht_del_item(ht_item* it)
{
	free(it->key);
	free(it->val);
	free(it);
}

/* Delete an entire hash table */
void ht_del_hash_table(ht_hash_table* ht)
{
	for (int i = 0; i < ht->size; i++) {
		ht_item* item = ht->items[i];
		if (item != NULL)
			ht_del_item(item);
	}
	free(ht->items);
	free(ht);
}

/* The hashing function */
static int ht_hash(const char* str, const int prime, const int len)
{
	long hash = 0;
	const int len_str = strlen(str);
	for (int i = 0; i < len_str; i++) {
		hash += (long)pow(prime, len_str - (i + 1)) * str[i];
		hash = hash % len;
	}
	return (int)hash;
}

/* Mitigate collisions */
static int ht_get_hash(const char* str, const int len, const int attempt)
{
	const int hash_a = ht_hash(str, HT_PRIME_1, len);
	const int hash_b = ht_hash(str, HT_PRIME_2, len);
	return (hash_a + (attempt * (hash_b + 1))) % len;
}

/*
 * Resize the hash_table by creating a new one with desired size
 * and swapping them
 */
static void ht_resize(ht_hash_table* ht, const int base_size)
{
	if (base_size < HT_INIT_BASE_SIZE)
		return;

	ht_hash_table* new_ht = ht_new_sized(base_size);
	for (int i = 0; i < ht->size; i++) {
		ht_item* item = ht->items[i];
		if (item != NULL && item != &HT_DELETED_ITEM)
			ht_insert(new_ht, item->key, item->val);
	}

	ht->base_size = new_ht->base_size;
	ht->cntr = new_ht->cntr;

	const int tmp_size = ht->size;
	ht->size = new_ht->size;
	new_ht->size = tmp_size;

	ht_item** tmp_items = ht->items;
	ht->items = new_ht->items;
	new_ht->items = tmp_items;

	ht_del_hash_table(new_ht);
}

/* Add size - used by ht_insert() */
static void ht_resize_up(ht_hash_table* ht)
{
	const int new_size = ht->base_size * 2;
	ht_resize(ht, new_size);
}

/* Shrink size - used by ht_delete() */
static void ht_resize_down(ht_hash_table* ht)
{
	const int new_size = ht->base_size / 2;
	ht_resize(ht, new_size);
}
