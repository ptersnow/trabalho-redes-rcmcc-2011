/***************************************************************************
             cache.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

static void
perror_exit(const char *msg)
{
	perror(msg);
	_exit(1);
}

static int
lruc_hash(char *key, int key_length)
{
	int k, m, r, h;
  char* data = key;

	m = 0x5bd1e995;
  r = 24;
  h = cache->seed ^ key_length;

  while(key_length >= 4) {
    k = *(int *)data;
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    key_length -= 4;
  }
  
  switch(key_length) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
  };

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h % cache->hash_table_size;
}

static int
lruc_cmp_keys(lruc_item *item, char *key, int key_length)
{
  if(key_length != item->key_length)
    return 1;
  else
    return memcmp(key, item->key, key_length);
}

static void
lruc_remove_item(lruc_item *prev, lruc_item *item, int hash_index)
{
  if(prev)
    prev->next = item->next;
  else
    cache->items[hash_index] = (lruc_item *) item->next;
  
  /* free memory and update the free memory counter */
  cache->free_memory += item->value_length;
  free(item->value);
  free(item->key);
  
  /* push the item to the free items queue  */
  memset(item, 0, sizeof(lruc_item));
  item->next = cache->free_items;
  cache->free_items = item;
}

void
lruc_remove_lru_item()
{
  lruc_item *min_item = NULL, *min_prev = NULL;
  lruc_item *item = NULL, *prev = NULL;
  int i = 0, min_index = -1;
  int min_access_count = -1;
  
  for(; i < cache->hash_table_size; i++) {
    item = cache->items[i];
    prev = NULL;
    
    while(item) {
      if(item->access_count < min_access_count || min_access_count == -1) {
        min_access_count = item->access_count;
        min_item  = item;
        min_prev  = prev;
        min_index = i;
      }
      prev = item;
      item = item->next;
    }
  }
  
  if(min_item)
    lruc_remove_item(min_prev, min_item, min_index);
}

static lruc_item *
lruc_pop_or_create_item()
{
  lruc_item *item = NULL;
  
  if(cache->free_items) {
    item = cache->free_items;
    cache->free_items = item->next;
  } 
  else item = (lruc_item *) calloc(sizeof(lruc_item), 1);
  
  return item;
}

void
lruc_new(int average_length)
{
  cache = (lruc *) calloc(sizeof(lruc), 1);
  if(!cache)
    perror_exit("Cache");
    
  cache->hash_table_size      = total_memory / average_length;
  cache->free_memory          = total_memory;
  cache->seed                 = time(NULL);
  
  /* size the hash table to a guestimate of the number of slots required (assuming a perfect hash) */
  cache->items = (lruc_item **) calloc(sizeof(lruc_item *), cache->hash_table_size);
  if(!cache->items) {
    free(cache);
    perror_exit("Cache items");
  }
  
  sem_init(&cache->lock, 0, 1);
}


void
lruc_free()
{
  /* free each of the cached items, and the hash table */
  int i = 0;
  lruc_item *item = NULL, *next = NULL;
  
  if(cache->items) {
    for(; i < cache->hash_table_size; i++) {
      item = cache->items[i];    
      while(item) {
      	munmap(item->value, item->value_length);
        next = item->next;
        free(item);
        item = next;
      }
    }
    free(cache->items);
  }
  
  sem_destroy(&cache->lock);
  
  free(cache);
}


int
lruc_set(char *key, int key_length, char *value, int value_length)
{
	int fd;
	char *path, *aux = NULL;
  int hash_index = lruc_hash(key, key_length), required = 0;
  lruc_item *item = NULL, *prev = NULL;
  
  
	asprintf(&path, "%s/data_%d", cache_directory, hash_index);
	
  if((fd = open(path, O_RDWR | O_CREAT | O_APPEND, S_IRWXU)) == -1) {
		perror("lruc_set open");
		return 1;
  }
  
  if(value) {
		write(fd, value, value_length);
		aux = mmap(0, value_length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
	}
  close(fd);
  
  sem_wait(&cache->lock);
  
  item = cache->items[hash_index];
  while(item && lruc_cmp_keys(item, key, key_length)) {
    prev = item;
    item = item->next;
  }
  
  if(item) {
    /* update the value and value_lengths */
    if(aux) {
			required = value_length - item->value_length;
			munmap(item->value, item->value_length);
			item->value = aux;
			item->value_length = value_length;
		} 
  }
  else {
    /* insert a new item */
    item = lruc_pop_or_create_item();
    item->value = NULL;
    item->key = key;
    item->value_length = 0;
    item->key_length = key_length;
    required = 0;
    
    if(prev)
      prev->next = item;
    else
      cache->items[hash_index] = item;
  }
  item->access_count = ++cache->access_count;
  
  /* remove as many items as necessary to free enough space */
  if(required > 0 && required > cache->free_memory) {
    while(cache->free_memory < required)
      lruc_remove_lru_item();
  }
  cache->free_memory -= required;
  sem_post(&cache->lock);
  
  return 0;
}


char *
lruc_get(char *key, int key_length)
{  
  /* loop until we find the item, or hit the end of a chain */
  lruc_item *item = cache->items[lruc_hash(key, key_length)];
  
  sem_wait(&cache->lock);
  while(item && lruc_cmp_keys(item, key, key_length))
    item = item->next;
  
  if(item && (item->value_length != 0)) {
    item->access_count = ++cache->access_count;
    sem_post(&cache->lock);
    return item->value;
  } 

	sem_post(&cache->lock);
  return NULL;
}


void
lruc_delete(char *key, int key_length)
{ 
  int hash_index = lruc_hash(key, key_length);
  lruc_item *item = NULL, *prev = NULL;
  
  sem_wait(&cache->lock);
  item = cache->items[hash_index];
  while(item && lruc_cmp_keys(item, key, key_length)) {
    prev = item;
    item = item->next;
  }
  
  if(item)
    lruc_remove_item(prev, item, hash_index);
  
  sem_post(&cache->lock);
}
