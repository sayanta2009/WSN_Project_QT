#define INFINITE_HOP_COUNT 0xFF
#define MAX_NEIGHBORS 10
#define TIME_TO_LIVE 50

#define COST_SHARE_INTERVAL (5)
#define RANDOM_RAND_MAX 65535U

// MAC LAYER PARAMETERS
#define NETSTACK_CONF_MAC csma_driver
#define NETSTACK_CONF_RDC nullrdc_driver
//#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 256

typedef struct
{
	struct neighbors *next;
	linkaddr_t address;
	unsigned int cost;
	struct ctimer node_timer;
}neighbor;

typedef struct
{
	linkaddr_t addr;
	unsigned int cost;
} nodes;


typedef struct{
	unsigned int hops;
	char text[30];
	char type[4];
	int16_t rssi;
	char path[25];
}route_packet;

