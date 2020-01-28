/*
 * API for the hash table manipulation.
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <string.h>
#include <stddef.h>

typedef struct {
	char* key;
	char* val;
} ht_item;

typedef struct {
	int base_size;
	int size;
	int cntr;
	ht_item** items;
} ht_hash_table;

/* Initial base_size of a table */
static int HT_INIT_BASE_SIZE = 53;

/* Item marked as deleted */
static ht_item HT_DELETED_ITEM = {NULL, NULL};

/* Any prime above 128 (ASCII table size) will do */
static int HT_PRIME_1 = 151;
static int HT_PRIME_2 = 163;

/* hash_table.h */
void ht_insert(ht_hash_table* ht, const char* key, const char* val);
char* ht_search(ht_hash_table* ht, const char* key);
void ht_delete(ht_hash_table* ht, const char* key);

/* hash_table.c */
static ht_item* ht_new_item(const char* key, const char* val);
static ht_hash_table* ht_new_sized(const int base_size);
ht_hash_table* ht_new_table(void);
static void ht_del_item(ht_item* it);
void ht_del_hash_table(ht_hash_table* ht);
static int ht_hash(const char* key, const int prime, const int len);
static int ht_get_hash(const char* key, const int prime, const int len);
static void ht_resize(ht_hash_table* ht, const int base_size);
static void ht_resize_up(ht_hash_table* ht);
static void ht_resize_down(ht_hash_table* ht);

/*----------------------------------------------------------------------------*/

/* Insert an item into hash table */
void ht_insert(ht_hash_table* ht, const char* key, const char* val)
{
	/* Check if table needs resizing */
	const int load = ht->cntr * 100 / ht->size;
	if (load > 70)
		ht_resize_up(ht);

	ht_item* item = ht_new_item(key, val);
	int index = ht_get_hash(item->key, ht->size, 0);
	ht_item* cur_item = ht->items[index];
	int i = 1;
	while (cur_item != NULL) {
		if (cur_item != &HT_DELETED_ITEM) {
			if (strcmp(cur_item->key, key) == 0) {
				ht_del_item(cur_item);
				ht->items[index] = item;
				return;
			}
		}
		index = ht_get_hash(item->key, ht->size, i);
		cur_item = ht->items[index];
		i++;
	}
	ht->items[index] = item;
	ht->cntr++;
}

/* Search for an item */
char* ht_search(ht_hash_table* ht, const char* key)
{
	int index = ht_get_hash(key, ht->size, 0);
	ht_item* item = ht->items[index];
	int i = 1;
	while (item != NULL) {
		if (item != &HT_DELETED_ITEM) {
			if (strcmp(item->key, key) == 0) {
				return item->val;
			}
		}
		index = ht_get_hash(key, ht->size, i);
		item = ht->items[index];
		i++;
	}
	return NULL;
}

/* Delete an item */
void ht_delete(ht_hash_table* ht, const char* key)
{
	/* Check if table needs resizing */
	const int load = ht->cntr * 100 / ht->size;
	if (load < 10)
		ht_resize_down(ht);

	int index = ht_get_hash(key, ht->size, 0);
	ht_item* item = ht->items[index];
	int i = 1;
	while (item != NULL) {
		if (item != &HT_DELETED_ITEM) {
			if (strcmp(item->key, key) == 0) {
				ht_del_item(item);
				ht->items[index] = &HT_DELETED_ITEM;
			}
		}
		index = ht_get_hash(key, ht->size, i);
		item = ht->items[index];
		i++;
	}
	ht->cntr--;
}

#endif // HASH_TABLE_H_
