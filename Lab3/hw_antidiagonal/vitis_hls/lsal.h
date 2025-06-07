#define N 32
#define M 65536
#define PADDED_M (M + 2 * (N - 1))

typedef enum { CENTER, NORTH_WEST, NORTH, WEST } Direction;
#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_DIAG_LEN MIN(N, M)
#define MEMCPY_CHAR_BW 64

#define DIAGONAL_LENGTH(d, max_dim, min_dim) ((d) >= (max_dim) ? (min_dim) + (max_dim - d) : (d < min_dim ? d : min_dim))
#define DIAG_ARRAY_LEN 3 * MAX_DIAG_LEN + 3
