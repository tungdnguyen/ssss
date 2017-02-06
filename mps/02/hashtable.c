#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  // FIXME: the current implementation doesn't update existing entries
  unsigned int idx = hash(key)%(ht->size);
  bucket_t *b = ht -> buckets[idx];
  while(b){
    if (strcmp(b->key,key)==0)
      {
	free(key);
	free(b->val);
	b->val = val;
	return;
      }
    b = b->next;
  }

  b = malloc(sizeof(bucket_t));
  b->key = key;
  b->val = val;
  b->next = ht->buckets[idx];
  ht->buckets[idx] = b;
 }

void *ht_get(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];

  while (b) {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    bucket_t *b = ht->buckets[i];
    while (b) {
      bucket_t *temp = b;
      b = b->next;
      free(temp->key);
      free(temp->val);
      free(temp);
    }
  }
  free(ht->buckets);
  free(ht);
}
/* TODO */
void  ht_del(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key)%ht->size;
  bucket_t *b = ht->buckets[idx];
  bucket_t *curr = b;
  if(strcmp(b->key,key)==0){
    ht->buckets[idx]=b->next;
    free(b->key);
    free(b->val);
    free(b);
    return;
  }
  else{
    b=b->next;
}
  while(b) {
    if(strcmp(b->key,key)==0){
      curr->next = b->next;
      free(b->key);
      free(b->val);
      free(b);
      
    }
    curr=b;
    b=b->next;
  }
}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {
  hashtable_t *newht = make_hashtable(newsize);
  bucket_t *b;
  unsigned long i;
  for(i =0; i<ht->size;i++){
    b=ht->buckets[i];
    while(b){
      ht_put(newht,b->key,b->val);
      bucket_t *temp2 = b;
      b=b->next;
      free(temp2);
  }
  }
  free(ht->buckets);
  *ht=*newht;
  free(newht);
}
