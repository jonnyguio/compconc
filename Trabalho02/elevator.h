#define MAX_ELEVATORS 100
#define MAX_FLOORS 100
#define MAX_CAPACITY 20

typedef struct _params {
    int id, floor, capacity;
} params;

typedef struct _queue {
    int size, array[MAX_CAPACITY];
} queue;

typedef struct _req {
    int id, size, inUse;
    queue people;
} req;

extern int N, M, C;
extern req *floorsReqs;

void* elevator(void* args);
int getFloorsFree();

void initQueue(queue *q);
void insertQ(queue *q, int val);
int removeQ(queue *q);
void printQueue(queue *q);
void freeQueue(queue *q);

extern pthread_mutex_t *floorsMutex;
