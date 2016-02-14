#define MAX_ELEVATORS 100
#define MAX_FLOORS 100
#define MAX_CAPACITY 20
#define MAX_PEOPLE 40
#define TAG_DEBUG 0

typedef struct _params {
    int id, f, capacity;
} params;

typedef struct _req {
    int id, size, inUse, people[MAX_PEOPLE];
} req;

extern int N, M, C, finishedInputs;
extern req *floorsReqs;

void* elevator(void* args);
int getFloorsFree();

void insertQ(req *q, int val);
int removeQ(req *q);
void printReq(req *q);
void freeReq(req *q);

extern pthread_mutex_t floorsMutex[MAX_FLOORS];
extern pthread_mutex_t teste;
