#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

void populate_hashtable(hashtable_t *ht) {
  char word[100];
  FILE *infile = fopen("/usr/share/dict/words", "r");
  int len, i;
  char *key, *val;

  while (fgets(word, sizeof(word), infile)) {
    len = strlen(word);
    if (word[len-1] == '\n') {
      word[len-1] = '\0';
      len--;
    }
    key = malloc(len+1);
    val = malloc(len+1);
    strcpy(key, word);
    for (i=0; i<len; i++) {
      val[i] = key[len-i-1];
    }
    val[len] = '\0';
    ht_put(ht, key, val);
  }
}

int print_iter(char *key, void *val) {
  printf("%s -> %s\n", key, (char *)val);
  return 1;
}

void read_key_vals(hashtable_t *ht) {
  char buf[80], *p, *key, *val;
  printf("Enter key value pairs (empty line to end)\n");
  while (fgets(buf, sizeof(buf), stdin)) {
    if (strcmp(buf, "\n") == 0) {
      break;
    }
    p = strtok(buf, " ,");
    key = strdup(p);
    p = strtok(NULL, "\n");
    val = strdup(p);
    printf("Inserting %s=>%s\n", key, val);
    ht_put(ht, key, val);
  }
}

void query_keys(hashtable_t *ht) {
  char buf[80], *key, *val;
  printf("Enter keys to look up (empty line to end)\n");
  while (fgets(buf, sizeof(buf), stdin)) {
    if (strcmp(buf, "\n") == 0) {
      break;
    }
    key = strtok(buf, "\n");
    val = ht_get(ht, key);
    if (val) {
      printf("Found %s=>%s\n", key, val);
    } else {
      printf("Key %s not found\n", key);
    }
  }
}

int main(int argc, char *argv[]) {
  hashtable_t *ht;

  if (argc < 2) {
    printf("Usage: %s NUM_HASH_BUCKETS\n", argv[0]);
    exit(0);
  }

  ht = make_hashtable(atoi(argv[1]));
  read_key_vals(ht);
  printf("Entries:\n");
  ht_iter(ht, print_iter);
  printf("\nMax chain length = %d\n\n", ht_max_chain_length(ht));
  query_keys(ht);
  free_hashtable(ht);
  return 0;
}
