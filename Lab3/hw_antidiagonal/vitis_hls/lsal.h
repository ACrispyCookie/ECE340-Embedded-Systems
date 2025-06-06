#define N 32
#define M 65536

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_DIAG_LEN MIN(N, M)
#define MEMCPY_CHAR_BW 64

