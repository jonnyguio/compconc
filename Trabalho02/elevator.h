// CONSTANTES USADAS AO LONGO DO PROGRA A

#define MAX_ELEVATORS 100
#define MAX_FLOORS 100
#define MAX_CAPACITY 20
#define MAX_PEOPLE 40
#define TAG_DEBUG 0 // debug ligado permite ver mais detalhadamente o código (porém muito confuso)

// ESTRUTURAS UTILIZADAS NO PROGRAMA
typedef struct _params {
    int id, f, capacity;
} params;

typedef struct _req {
    int id, size, inUse, people[MAX_PEOPLE];
} req;

// VARIAVEIS GLOBAIS
extern int N, M, C, finishedInputs;
extern req floorsReqs[MAX_FLOORS];

extern pthread_mutex_t floorsMutex[MAX_FLOORS];
extern pthread_mutex_t teste;

// FUNÇÕES GLOBAIS
void* elevator(void* args);
int getFloorsFree();

void insertQ(req *q, int val);
int removeQ(req *q);
void printReq(req *q);
void freeReq(req *q);
