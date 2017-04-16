#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#define BUFFER_SIZE 1024
typedef struct{
  int valid;
  int tag;
  int timestamp;
}line_t;
typedef struct{
  line_t *lines;
}set_t;

typedef struct{
  int s;
  int b;
  int E;
  set_t *sets;
}cache_t;

typedef struct{
  int hit;
  int miss;
  int eviction;
}result_t;

void printHelpMessage(char *argv[]);
void lineProcess(cache_t *cache,result_t *result,unsigned long int memory_address,int verbose);
void changeTimestamp(cache_t *cache, int set_index, int E);

int main(int argc, char *argv[]){

  cache_t cache;
  result_t result;
  result.hit = 0, result.miss = 0, result.eviction = 0;
  int arg,verbose=0;
  cache.s = 0,cache.E = 0,cache.b = 0;
  char *trace;
  while ((arg = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch(arg) {
    case 'h':
      printHelpMessage(argv);
      break;
    case 'v':
      verbose = 1;
      break;
    case 's':
      cache.s = atoi(optarg);
      break;
    case 'E':
      cache.E = atoi(optarg);
      break;
    case 'b':
      cache.b = atoi(optarg);
      break;
    case 't':
      trace = optarg;
      break;
    default:
      exit(1);
    }
  }

  //cache initialize
  int number_of_set = pow(2,cache.s);
  cache.sets = (set_t *)malloc(number_of_set*sizeof(set_t));
  for (int i =0; i <number_of_set;i++)
    {
      cache.sets[i].lines=(line_t *)malloc(cache.E*sizeof(line_t));
    }

  for (int i=0;i<number_of_set;i++)
    {
      for(int j =0;j<cache.E;j++)
        {
	  cache.sets[i].lines[j].valid = 0;
	}
    }

  unsigned long int memory_address;
  int size;
  char command;

  //read in the file and start executing
  FILE *readFile  = fopen(trace,"r");
  if(readFile == NULL){
    printf("cannot open file\n");
  }
  else
    {
      char str[BUFFER_SIZE];
      while(fgets(str,BUFFER_SIZE,readFile)!=NULL)
        {

	  if(str[0]!='I')
	    {
	      if(verbose == 1)
		{
		  printf("%s",str);
		}
	      sscanf(str+2,"%lx,%x",&memory_address,&size);
	      command = str[1];
	      if(command == 'L' || command == 'S')
		{
		  lineProcess(&cache,&result,memory_address,verbose);
		  printf("\n");
		}
	      else if(str[1] == 'M')
		{
		  lineProcess(&cache,&result,memory_address,verbose);
		  printf("\n");
		  lineProcess(&cache,&result,memory_address,verbose);
		  printf("\n");
		}
	    }
	}
      fclose(readFile);

    }

  printSummary(result.hit,result.miss,result.eviction);
}

void lineProcess(cache_t *cache,result_t *result,unsigned long int memory_address,int verbose)
{
  //int time = 0;
  //printf("address %lx",memory_address);
  unsigned long int tag = memory_address >> (cache->s+cache->b);
  unsigned long int temp = (1<<cache->s)-1;
  unsigned long int set_index = (memory_address>>cache->b)&temp;
  //check for a hit
  for (int i =0;i<cache->E;i++)
    {
      //if hit 
      if((cache->sets[set_index].lines[i].tag==tag)&&(cache->sets[set_index].lines[i].valid == 1))
	{
	  result->hit ++;
	  //cache->sets[set_index].lines[i].timestamp = time;
	  //time ++;
	  if(verbose == 1)
	    {
	      printf("Hit ");
	    }
	  changeTimestamp(cache,set_index,i);
	  return;
	}
    }
  //if miss
  result -> miss ++;
  if( verbose == 1)
    {
      printf("Miss ");
    }
  //if we don't need to evict => choose the first one that's available
  for(int i = 0; i<cache->E;i++)
    {
      if(cache->sets[set_index].lines[i].valid == 0)
	{
	  cache->sets[set_index].lines[i].valid = 1;
	  cache->sets[set_index].lines[i].tag = tag;
	  //cache->sets[set_index].lines[i].timestamp = time;
	  //time++;
	  changeTimestamp(cache,set_index,i);
	  return;

	}
    }
  //If we cannot find a good space, we will need to evict
  result -> eviction++;
  if(verbose == 1)
    {
      printf("Evict ");
    }

  //Let's evict
  for (int i = 0; i < cache->E; i++)
    {
      if (cache->sets[set_index].lines[i].timestamp == 1)
	{
	  cache->sets[set_index].lines[i].valid = 1;
	  cache->sets[set_index].lines[i].tag = tag;
	  changeTimestamp(cache,set_index, i);
	  return;
	}
    }
  //looking for oldest item
  //old way of making time stamp, somehow it messes up
  //int lowest=cache->sets[set_index].lines[0].timestamp;
  //int index_of_lowest=0;
  //for (int i =1; i<cache->E;i++)
  //{
  //if(cache->sets[set_index].lines[i].timestamp<lowest)
  //{
  //lowest = cache->sets[set_index].lines[i].timestamp;
  //index_of_lowest = i;
  //} 
  //}
  //let's evict
  //cache->sets[set_index].lines[index_of_lowest].tag = tag;
  //cache->sets[set_index].lines[index_of_lowest].timestamp=time;
  //time++;
  return;
}

void changeTimestamp(cache_t *cache, int set_index, int E) 
{
  int i;
  for (i = 0; i<cache->E; i++) 
    {
      if ((cache->sets[set_index].lines[i].valid)&&(cache->sets[set_index].lines[i].timestamp > cache->sets[set_index].lines[E].timestamp)) 
	{
	  cache->sets[set_index].lines[i].timestamp--;
	}
    }
  cache->sets[set_index].lines[E].timestamp = cache->E;
  return;
}

void printHelpMessage(char *argv[]){
  printf("\n");
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n\n");
  printf("Examples:\n");
  printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
  printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
  printf("\n");
  return;
} 
