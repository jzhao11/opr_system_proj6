#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define MAX_CMD_LEN 256
#define PARAM_NUM_ERR 1
#define MEM_ALLOC_ERR 2
#define RED_CLR "\x1b[31m"
#define GREEN_CLR "\x1b[32m"
#define YELLOW_CLR "\x1b[33m"
#define BLUE_CLR "\x1b[34m"
#define MAGENTA_CLR "\x1b[35m"
#define CYAN_CLR "\x1b[36m"
#define RESET_CLR "\x1b[0m"

struct producer {
	int index;
	int tnum;
	pthread_t tid;
};

struct consumer {
	int index;
	int tnum;
	pthread_t tid;
};

int* bounded_buffer;
struct producer* producer_thread;
struct consumer* consumer_thread;
int* producer_arr;
int* consumer_arr;
int* producer_arr_ptr;
int* consumer_arr_ptr;
int buffer_size;
int producer_size;
int consumer_size;
int producer_item_num;
int consumer_item_num;
int over_item_num;
int producer_sleep;
int consumer_sleep;
int array_size;
sem_t full;
sem_t empty;
sem_t mutex;
int in = 0;
int out = 0;
int counter = 0;

/* 
 * Function to remove item.
 * Item removed is returned
 */
int dequeue_item() {
	int item = bounded_buffer[out];
	out = (out + 1) % buffer_size;
	return item;
}

/* 
 * Function to add item.
 * Item added is returned.
 * It is up to you to determine
 * how to use the ruturn value.
 * If you decide to not use it, then ignore
 * the return value, do not change the
 * return type to void. 
 */
int enqueue_item(int item) {
	bounded_buffer[in] = item;
	in = (in + 1) % buffer_size;
	return item;
}

// initialization and memory allocation
int init(char argc, char** argv) {
	if (argc != 7) {
		return PARAM_NUM_ERR;
	} else {
		buffer_size = atoi(argv[1]);
		producer_size = atoi(argv[2]);
		consumer_size = atoi(argv[3]);
		producer_item_num = atoi(argv[4]);
		producer_sleep = atoi(argv[5]);
		consumer_sleep = atoi(argv[6]);
		consumer_item_num = producer_size * producer_item_num / consumer_size;
		array_size = producer_size * producer_item_num;
		over_item_num = array_size % consumer_size;

		bounded_buffer = (int*) calloc(sizeof(int), buffer_size);
		producer_thread = (struct producer*) calloc(sizeof(struct producer), producer_size);
		consumer_thread = (struct consumer*) calloc(sizeof(struct consumer), consumer_size);
		producer_arr = (int*) calloc(sizeof(int), array_size);
		consumer_arr = (int*) calloc(sizeof(int), array_size);
		producer_arr_ptr = producer_arr;
		consumer_arr_ptr = consumer_arr;

		if (bounded_buffer == NULL || producer_thread == NULL 
			|| consumer_thread == NULL || producer_arr == NULL 
			|| consumer_arr == NULL) {
			return MEM_ALLOC_ERR;
		}
	}

	return 0;
}

// destruction and memory deallocation
void destruct() {
	free(bounded_buffer);
	free(producer_thread);
	free(consumer_thread);
	free(producer_arr);
	free(consumer_arr);
}

// retrieve timestamp as char*
char* datetime(time_t* timer) {
	time(timer);
	struct tm* timeinfo = localtime(timer);
	return asctime(timeinfo);
}

// producer thread function
void* producer_func(void* param) {
	struct producer pitem = *((struct producer*) param);
	int tmp;
	for (int i = 0; i < pitem.tnum; ++i) {
		sem_wait(&empty);
		sem_wait(&mutex);
		counter++;
		tmp = enqueue_item(counter);
		*producer_arr_ptr = tmp;
		producer_arr_ptr++;
		sem_post(&mutex);
		sem_post(&full);
		printf(BLUE_CLR);
		printf("%d\twas produced by producer->\t%d\n", tmp, pitem.index);
		printf(RESET_CLR);
		sleep(producer_sleep);
	}
	pthread_exit(0);
}

// consumer thread function
void* consumer_func(void* param) {
	struct consumer citem = *((struct consumer*) param);
	int tmp;
	for (int i = 0; i < citem.tnum; ++i) {
		sem_wait(&full);
		sem_wait(&mutex);
		tmp = dequeue_item();
		*consumer_arr_ptr = tmp;
		consumer_arr_ptr++;
		sem_post(&mutex);
		sem_post(&empty);
		printf(GREEN_CLR);
		printf("%d\twas consumed by consumer->\t%d\n", tmp, citem.index);
		printf(RESET_CLR);
		sleep(consumer_sleep);
	}
	pthread_exit(0);
}

int main(int argc, char** argv) {
	// initialization
	int pc_match_flag = 1;
	time_t start_at;
	time_t end_at;

	int init_rtn = init(argc, argv);
	if (init_rtn == PARAM_NUM_ERR) {
		printf("Error in Number of Parameters!\n");
		printf("Valid Command: ./pandc N P C X Ptime Ctime\n");
		printf("N: number of buffers\n");
		printf("P: number of producer threads\n");
		printf("C: number of consumer threads\n");
		printf("X: number of items produced by each producer thread\n");
		printf("Ptime: time each producer sleeps (in seconds)\n");
		printf("Ctime: time each consumer sleeps (in seconds)\n");
		return init_rtn;
	} else if (init_rtn == MEM_ALLOC_ERR) {
		printf("Error in Memory Allocation!\n");
		return init_rtn;
	}

	// semaphores
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, buffer_size);
	sem_init(&mutex, 0, 1);

	// start of timer
	printf("Current Time: %s", datetime(&start_at));

	// threads creation
	for (int i = 0; i < producer_size; ++i) {
		producer_thread[i].tnum = producer_item_num;
		producer_thread[i].index = i + 1;
		pthread_create(&producer_thread[i].tid, NULL, producer_func, &producer_thread[i]);
	}
	for (int i = 0; i < consumer_size; ++i) {
		if (i == 0) {
			consumer_thread[i].tnum = consumer_item_num + over_item_num;
		} else {
			consumer_thread[i].tnum = consumer_item_num;
		}
		consumer_thread[i].index = i + 1;
		pthread_create(&consumer_thread[i].tid, NULL, consumer_func, &consumer_thread[i]);
	}

	// output of parameters
	printf("Number of Buffers: %d\n", buffer_size);
	printf("Number of Producers: %d\n", producer_size);
	printf("Number of Consumers: %d\n", consumer_size);
	printf("Number of Items Produced by Each Producer: %d\n", producer_item_num);
	printf("Number of Items Consumed by Each Consumer: %d\n", consumer_item_num);
	printf("Over-consumption on: %d\n", over_item_num != 0);
	if (over_item_num) {
		printf("Over-consumption Amount: %d\n", consumer_thread[0].tnum);
	}
	printf("Time Each Producer Sleeps (seconds): %d\n", producer_sleep);
	printf("Time Each Consumer Sleeps (seconds): %d\n", consumer_sleep);

	// threads join
	for (int i = 0; i < producer_size; ++i) {
		pthread_join(producer_thread[i].tid, NULL);
		printf(YELLOW_CLR);
		printf("Producer Thread Joined: %d\n", producer_thread[i].index);
		printf(RESET_CLR);
	}
	for (int i = 0; i < consumer_size; ++i) {
		pthread_join(consumer_thread[i].tid, NULL);
		printf(RED_CLR);
		printf("Consumer Thread Joined: %d\n", consumer_thread[i].index);
		printf(RESET_CLR);
	}

	// end of timer
	printf("Current Time: %s", datetime(&end_at));

	// output of producer and consumer arrays
	printf("Producer Array\t| Consumer Array\n");
	for (int i = 0; i < array_size; ++i) {
		if (producer_arr[i] != consumer_arr[i]) {
			pc_match_flag = 0;
		}
		printf("%d\t\t| %d\n", producer_arr[i], consumer_arr[i]);
	}
	if (pc_match_flag) {
		printf("Consumer and Producer Arrays Match!\n");
	}
	printf("Total Runtime: %ld secs\n", end_at - start_at);

	// destruction
	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&mutex);
	destruct();

	return 0;
}
