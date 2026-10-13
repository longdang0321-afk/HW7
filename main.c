#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#define N 100
#define PAIRS (N * (N - 1) / 2)

typedef struct Node {
    int v;
    struct Node *next;
} Node;

typedef struct Result {
    size_t memory;
    long insertion, deletion, connectivity, adjacent;
    int connected, count, neighbors[N];
} Result;

/* 맨 앞에 삽입하므로 위치 탐색은 필요 없다. */
static void push(Node **head, int v)
{
    Node *node = malloc(sizeof *node);
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->v = v;
    node->next = *head;
    *head = node;
}

static int find(Node *head, int v, long *comparisons)
{
    for (; head != NULL; head = head->next) {
        (*comparisons)++;
        if (head->v == v) return 1;
    }
    return 0;
}

/* 양쪽 리스트에서 각각 호출하여 무방향 간선을 삭제한다. */
static int unlink_node(Node **head, int v, long *comparisons)
{
    for (; *head != NULL; head = &(*head)->next) {
        (*comparisons)++;
        if ((*head)->v == v) {
            Node *old = *head;
            *head = old->next;
            free(old);
            return 1;
        }
    }
    return 0;
}

/* 검증에 사용한 확인 횟수는 성능 측정에 포함하지 않는다. */
static void check(int matrix[N][N], Node *adj[N], int edges)
{
    long total = 0;
    for (int u = 0; u < N; u++) {
        int seen[N] = {0};
        int degree = 0;
        assert(matrix[u][u] == 0);
        for (int v = 0; v < N; v++) {
            assert(matrix[u][v] == 0 || matrix[u][v] == 1);
            assert(matrix[u][v] == matrix[v][u]);
        }
        for (Node *p = adj[u]; p != NULL; p = p->next) {
            assert(p->v >= 0 && p->v < N);
            assert(p->v != u && seen[p->v] == 0);
            seen[p->v] = 1;
            degree++;
        }
        for (int v = 0; v < N; v++) assert(seen[v] == matrix[u][v]);
        total += degree;
    }
    assert(total == 2L * edges);
}

static Result matrix_operations(int matrix[N][N], int iu, int iv,
                                int du, int dv, int cu, int cv, int x)
{
    Result r = {0};
    r.memory = sizeof(int[N][N]);
    r.insertion++;
    if (matrix[iu][iv] == 0) matrix[iu][iv] = matrix[iv][iu] = 1;
    r.deletion++;
    if (matrix[du][dv] == 1) matrix[du][dv] = matrix[dv][du] = 0;
    r.connectivity++;
    r.connected = matrix[cu][cv];
    for (int v = 0; v < N; v++) {
        r.adjacent++;
        if (matrix[x][v]) r.neighbors[r.count++] = v;
    }
    return r;
}

static Result list_operations(Node *adj[N], int edges, int iu, int iv,
                              int du, int dv, int cu, int cv, int x)
{
    Result r = {0};
    r.memory = sizeof(Node *[N]) + 2 * (size_t)edges * sizeof(Node);
    if (!find(adj[iu], iv, &r.insertion)) {
        push(&adj[iu], iv);
        push(&adj[iv], iu);
    }
    /* assert 안에서 삭제하지 않아 NDEBUG 여부와 관계없이 실행된다. */
    int a = unlink_node(&adj[du], dv, &r.deletion);
    int b = unlink_node(&adj[dv], du, &r.deletion);
    if (!a || !b) {
        fprintf(stderr, "Edge deletion failed\n");
        exit(EXIT_FAILURE);
    }
    r.connected = find(adj[cu], cv, &r.connectivity);
    for (Node *p = adj[x]; p != NULL; p = p->next) {
        r.adjacent++;
        r.neighbors[r.count++] = p->v;
    }
    return r;
}

static void print_result(int number, const char *name, const char *type,
                         const Result *r, int x)
{
    printf("Case %d : %s Graph - %s\n", number, name, type);
    printf("Memory                         : %zu Bytes\n", r->memory);
    printf("Edge insertion comparisons     : %ld\n", r->insertion);
    printf("Edge deletion comparisons      : %ld\n", r->deletion);
    printf("Connectivity check comparisons : %ld\n", r->connectivity);
    printf("Adjacent vertices comparisons  : %ld\n", r->adjacent);
    printf("Adjacent of %d (%d):", x, r->count);
    for (int i = 0; i < r->count; i++) printf(" %d", r->neighbors[i]);
    printf("\n\n");
}

static void run(const char *name, int edges, int first_case)
{
    int all[PAIRS][2]; /* 생성용 임시 배열: 그래프 메모리에서 제외 */
    int matrix[N][N] = {0};
    Node *adj[N] = {0};
    int k = 0;
    for (int u = 0; u < N; u++) {
        for (int v = u + 1; v < N; v++) {
            all[k][0] = u;
            all[k++][1] = v;
        }
    }
    for (int i = PAIRS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int u = all[i][0], v = all[i][1];
        all[i][0] = all[j][0]; all[i][1] = all[j][1];
        all[j][0] = u; all[j][1] = v;
    }
    for (int i = 0; i < edges; i++) {
        int u = all[i][0], v = all[i][1];
        matrix[u][v] = matrix[v][u] = 1;
        push(&adj[u], v); push(&adj[v], u);
    }
    check(matrix, adj, edges);
    int iu, iv, cu, cv;
    do {
        iu = rand() % N; iv = rand() % N;
    } while (iu == iv || matrix[iu][iv]);
    int d = rand() % edges;
    int du = all[d][0], dv = all[d][1];
    do {
        cu = rand() % N; cv = rand() % N;
    } while (cu == cv);
    int x = rand() % N;
    Result m = matrix_operations(matrix, iu, iv, du, dv, cu, cv, x);
    Result l = list_operations(adj, edges, iu, iv, du, dv, cu, cv, x);
    check(matrix, adj, edges);
    assert(m.connected == l.connected && m.count == l.count);
    /* 출력 순서는 달라도 인접 정점의 집합은 같아야 한다. */
    int seen[N] = {0};
    for (int i = 0; i < m.count; i++) seen[m.neighbors[i]]++;
    for (int i = 0; i < l.count; i++) seen[l.neighbors[i]]--;
    for (int v = 0; v < N; v++) assert(seen[v] == 0);
    printf("[%s Graph] Vertices : %d / Edges : %d\n", name, N, edges);
    printf("Insert (%d,%d) / Delete (%d,%d) / Connectivity (%d,%d) = %s\n",
           iu, iv, du, dv, cu, cv, m.connected ? "yes" : "no");
    print_result(first_case, name, "Adjacency Matrix", &m, x);
    print_result(first_case + 1, name, "Adjacency List", &l, x);
    for (int u = 0; u < N; u++) {
        while (adj[u] != NULL) {
            Node *old = adj[u]; adj[u] = old->next; free(old);
        }
    }
}

int main(int argc, char *argv[])
{
    unsigned int seed = 1;
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [seed]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        char *end;
        errno = 0;
        unsigned long value = strtoul(argv[1], &end, 10);
        if (argv[1][0] == '-' || end == argv[1] || *end != '\0' ||
            errno == ERANGE || value > UINT_MAX) {
            fprintf(stderr, "Seed must be an unsigned integer\n");
            return EXIT_FAILURE;
        }
        seed = (unsigned int)value;
    }
    srand(seed);
    printf("Seed : %u\n\n", seed);
    run("Sparse", 100, 1);
    run("Dense", 4000, 3);
    return EXIT_SUCCESS;
}
