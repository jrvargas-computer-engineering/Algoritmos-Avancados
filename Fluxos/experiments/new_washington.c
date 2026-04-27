#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define FAILURE    0
#define SUCCESS    1
#define FALSE      0
#define TRUE       1
#define MAX_CAP    100000000
#define UNDEFINED  0
#define MINCOSTFLOW 1
#define MAXFLOW    2
#define ASSIGNMENT 3
#define VERY_BIG   1000000

typedef struct enode {
  struct enode *next;
  struct enode *mate;
  int32_t c;
  int32_t f;
  int32_t h;
  int32_t t;
  int32_t flag;
} Edge;

typedef struct {
  Edge **A;
  int32_t *V;
  int32_t size;
  int32_t max_v;
} Graph;

typedef struct {
  int32_t head, tail, size;
  int32_t *data;
} Queue;

int32_t Range[] = {1000000, 500000, 250000, 125000, 62500, 31250,
         15625, 7812, 3906, 1953, 976, 488, 244, 122,
         61, 31, 15, 7, 4, 2};

/* --- Function Prototypes to fix "Implicit Declaration" warnings --- */
void InitRandom(int32_t seed);
void Barf(char *s);
int32_t RandomInteger(int32_t low, int32_t high);
void RandomSubset(int32_t low, int32_t high, int32_t n, int32_t *x);
void InitGraph(Graph *G, int32_t n);
void AddVertex(int32_t v, Graph *G);
void AddEdge(int32_t v1, int32_t v2, int32_t a, Graph *G);
int32_t NewVertex(Graph *G);
void Gadget(int32_t a, int32_t b, int32_t n, int32_t m, int32_t c, Graph *G, int32_t very_big);
void Bridge(int32_t a, int32_t b, int32_t n, Graph *G);
void Sink(int32_t k, Graph *G, int32_t very_big);
int32_t EdgeCount(Graph *G);
void GraphOutput(Graph *G, FILE *f, int32_t s, int32_t t);
void WriteVertex(int32_t v, Graph *G, FILE *f);
void WriteVertex2(int32_t v, Graph *G, FILE *f);
void WriteVertex3(int32_t v, Graph *G, FILE *f);
int32_t Abs(int32_t x);

/* --- Graph Generators --- */
Graph *Mesh(int32_t d1, int32_t d2, int32_t r);
Graph *RLevel(int32_t d1, int32_t d2, int32_t r);
Graph *R2Level(int32_t d1, int32_t d2, int32_t r);
Graph *Match(int32_t n, int32_t d);
Graph *SquareMesh(int32_t d, int32_t deg, int32_t r);
Graph *BasicLine(int32_t n, int32_t m, int32_t deg, int32_t range);
Graph *ExponentialLine(int32_t n, int32_t m, int32_t deg, int32_t range);
Graph *DExponentialLine(int32_t n, int32_t m, int32_t deg, int32_t range);
Graph *DinicBadCase(int32_t n);
Graph *GoldBadCase(int32_t n);
Graph *Cheryian(int32_t n, int32_t m, int32_t c, int32_t very_big);


int main(int argc, char *argv[]) {
  Graph *G = NULL;
  FILE *f;
  int32_t dim1, dim2, range, fct, s, t, deg;

  InitRandom(1);

  if (argc < 2) Barf("Usage: makegraph instance_type [specific parameters]");
  fct = atoi(argv[1]);

  if (fct >= 1 && fct <= 5) {
    if (argc != 6) Barf("Usage: makegraph [1;5] dim1 dim2 range file");
    dim1 = atoi(argv[2]); dim2 = atoi(argv[3]); range = atoi(argv[4]);
    if ((f = fopen(argv[5], "w")) == NULL) Barf("File Error");
  } else if (fct >= 6 && fct <= 8) {
    if (argc != 7) Barf("Usage: makegraph [6;8] n m deg range file");
    dim1 = atoi(argv[2]); dim2 = atoi(argv[3]); deg = atoi(argv[4]); range = atoi(argv[5]);
    if ((f = fopen(argv[6], "w")) == NULL) Barf("File Error");
  } else if (fct >= 9 && fct <= 10) {
    if (argc != 4) Barf("Usage: makegraph [9;10] n file");
    dim1 = atoi(argv[2]);
    if ((f = fopen(argv[3], "w")) == NULL) Barf("File Error");
  } else if (fct == 11) {
    if (argc != 7) Barf("Usage: makegraph 11 n m c very_big file");
    dim1 = atoi(argv[2]); dim2 = atoi(argv[3]); deg = atoi(argv[4]); range = atoi(argv[5]);
    if ((f = fopen(argv[6], "w")) == NULL) Barf("File Error");
  }

  switch(fct){
  case 1:
    fprintf(f, "c Mesh Graph\n");
    fprintf(f, "c %"PRId32" Rows, %"PRId32" columns, capacities in range [0, %"PRId32"]\n", dim1, dim2, range);
    G = Mesh(dim1, dim2, range); s = 0; t = G->size - 1; break;
  case 2:
    fprintf(f, "c Random Leveled Graph\n");
    fprintf(f, "c %"PRId32" Rows, %"PRId32" columns, capacities in range [0, %"PRId32"]\n", dim1, dim2, range);
    G = RLevel(dim1, dim2, range); s = 0; t = G->size - 1; break;
  case 3:
    fprintf(f, "c Random 2 Leveled Graph\n");
    fprintf(f, "c %"PRId32" Rows, %"PRId32" columns, capacities in range [0, %"PRId32"]\n", dim1, dim2, range);
    G = R2Level(dim1, dim2, range); s = 0; t = G->size - 1; break;
  case 4:
    fprintf(f, "c Matching Graph\n");
    fprintf(f, "c %"PRId32" vertices, %"PRId32" degree\n", dim1, dim2);
    G = Match(dim1, dim2); s = 0; t = G->size - 1; break;
  case 5:
    fprintf(f, "c Square Mesh\n");
    fprintf(f, "c %"PRId32" x %"PRId32" vertices, %"PRId32" degree, range [0,%"PRId32"]\n", dim1, dim1, dim2, range);
    G = SquareMesh(dim1, dim2, range); s = 0; t = G->size - 1; break;
  case 6:
    fprintf(f, "c Basic Line Mesh\n");
    fprintf(f, "c %"PRId32" x %"PRId32" vertices, degree %"PRId32", range [0,%"PRId32"]\n", dim1, dim2, deg, range);
    G = BasicLine(dim1, dim2, deg, range); s = 0; t = G->size - 1; break;
  case 7:
    fprintf(f, "c Exponential Line\n");
    fprintf(f, "c %"PRId32" x %"PRId32" vertices, degree %"PRId32", range [0,%"PRId32"]\n", dim1, dim2, deg, range);
    G = ExponentialLine(dim1, dim2, deg, range); s = 0; t = G->size - 1; break;
  case 8:
    fprintf(f, "c Double Exponential Line\n");
    fprintf(f, "c %"PRId32" x %"PRId32" vertices, degree %"PRId32", range [0,%"PRId32"]\n", dim1, dim2, deg, range);
    G = DExponentialLine(dim1, dim2, deg, range); s = 0; t = G->size - 1; break;
  case 9:
    fprintf(f, "c Line Graph - Bad case for Dinics\n");
    fprintf(f, "c %"PRId32" vertices\n", dim1);
    G = DinicBadCase(dim1); s = 0; t = G->size - 1; break;
  case 10:
    fprintf(f, "c  Bad case for Goldberg\n");
    fprintf(f, "c %"PRId32" vertices\n", dim1);
    G = GoldBadCase(dim1); s = 0; t = G->size - 1; break;
  case 11:
    fprintf(f, "c  Cheryian Graph\n");
    fprintf(f, "c n = %"PRId32", m = %"PRId32", c = %"PRId32", total vertices %"PRId32" \n", dim1, dim2, deg, 4*dim2*deg + 6 + 2*dim1);
    G = Cheryian(dim1, dim2, deg, range); s = 0; t = G->size - 1; break;
  default:
    Barf("Undefined class"); break;
  }

  GraphOutput(G, f, s, t);
  fclose(f);
  return 0;
}

Graph *Mesh(int32_t d1, int32_t d2, int32_t r) {
  Graph *G; int32_t i, j, source, sink;
  if (d1 < 2 || d2 < 2) Barf("Degenerate graph");
  if (d1*d2 + 2 > SIZE_MAX) Barf("Graph out of range");

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(d1*d2 + 2, sizeof(Edge *));
  G->V = calloc(d1*d2 + 2, sizeof(int32_t));
  InitGraph(G, d1*d2 + 2);  

  for (i = 0; i <= d1*d2 + 1; i++) AddVertex(i, G);
  source = 0; sink = d1*d2 + 1;

  for (i = 1; i <= d1; i++){
    AddEdge(source, source + i, 3*r, G);
    AddEdge(sink - i, sink, 3*r, G);
  }

  for (i = 0; i < d2 - 1; i++){
    AddEdge(i*d1 + 1, (i+1)*d1 + d1, RandomInteger(1, r), G);
    AddEdge(i*d1 + 1, (i+1)*d1 + 1, RandomInteger(1, r), G);
    AddEdge(i*d1 + 1, (i+1)*d1 + 2, RandomInteger(1, r), G);
    for (j = 2; j <= d1 - 1; j++){
      AddEdge(i*d1 + j, (i+1)*d1 + j - 1, RandomInteger(1, r), G);
      AddEdge(i*d1 + j, (i+1)*d1 + j, RandomInteger(1, r), G);
      AddEdge(i*d1 + j, (i+1)*d1 + j + 1, RandomInteger(1, r), G);
    }
    AddEdge(i*d1 + d1, (i+1)*d1 + d1 - 1, RandomInteger(1, r), G);
    AddEdge(i*d1 + d1, (i+1)*d1 + d1, RandomInteger(1, r), G);
    AddEdge(i*d1 + d1, (i+1)*d1 + 1, RandomInteger(1, r), G);
  }
  return G;
}

Graph *RLevel(int32_t d1, int32_t d2, int32_t r) {
  Graph *G; int32_t i, j, source, sink, x[3];
  if (d1 < 2 || d2 < 2) Barf("Degenerate graph");
  if (d1*d2 + 2 > SIZE_MAX) Barf("Graph out of range");

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(d1*d2 + 2, sizeof(Edge *));
  G->V = calloc(d1*d2 + 2, sizeof(int32_t));
  InitGraph(G, d1*d2 + 2);  

  for (i = 0; i <= d1*d2 + 1; i++) AddVertex(i, G);
  source = 0; sink = d1*d2 + 1;

  for (i = 1; i <= d1; i++){
    AddEdge(source, source + i, 3*r, G);
    AddEdge(sink - i, sink, 3*r, G);
  }

  for (i = 0; i < d2 - 1; i++){
    for (j = 1; j <= d1; j++){    
      RandomSubset(1, d1, 3, x);
      AddEdge(i*d1 + j, (i+1)*d1 + x[0], RandomInteger(1, r), G);
      AddEdge(i*d1 + j, (i+1)*d1 + x[1], RandomInteger(1, r), G);
      AddEdge(i*d1 + j, (i+1)*d1 + x[2], RandomInteger(1, r), G);
    }
  }
  return G;
}

Graph *R2Level(int32_t d1, int32_t d2, int32_t r) {
  Graph *G; int32_t i, j, source, sink, x[3];
  if (d1 < 2 || d2 < 2) Barf("Degenerate graph");

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(d1*d2 + 2, sizeof(Edge *));
  G->V = calloc(d1*d2 + 2, sizeof(int32_t));
  InitGraph(G, d1*d2 + 2);  

  for (i = 0; i <= d1*d2 + 1; i++) AddVertex(i, G);
  source = 0; sink = d1*d2 + 1;

  for (i = 1; i <= d1; i++){
    AddEdge(source, source + i, 3*r, G);
    AddEdge(sink - i, sink, 3*r, G);
  }

  for (i = 0; i < d2 - 2; i++){
    for (j = 1; j <= d1; j++){    
      RandomSubset(1, 2*d1, 3, x);
      AddEdge(i*d1 + j, (i+1)*d1 + x[0], RandomInteger(1, r), G);
      AddEdge(i*d1 + j, (i+1)*d1 + x[1], RandomInteger(1, r), G);
      AddEdge(i*d1 + j, (i+1)*d1 + x[2], RandomInteger(1, r), G);
    }
  }
  for (j = 1; j <= d1; j++){    
    RandomSubset(1, d1, 3, x);
    AddEdge((d2-2)*d1 + j, (d1-1)*d1 + x[0], RandomInteger(1, r), G);
    AddEdge((d2-2)*d1 + j, (d1-1)*d1 + x[1], RandomInteger(1, r), G);
    AddEdge((d2-2)*d1 + j, (d1-1)*d1 + x[2], RandomInteger(1, r), G);
  }
  return G;
}

Graph *Match(int32_t n, int32_t d) {
  Graph *G; int32_t i, j, source, sink;
  int32_t *x = calloc(2*n + 2, sizeof(int32_t)); // Removed 'const'

  if (n < 2 || d > n) Barf("Degenerate graph");

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(2*n + 2, sizeof(Edge *));
  G->V = calloc(2*n + 2, sizeof(int32_t));
  InitGraph(G, 2*n + 2);  

  for (i = 0; i <= 2*n + 1; i++) AddVertex(i, G);
  source = 0; sink = 2*n + 1;

  for (i = 1; i <= n; i++){
    AddEdge(source, source + i, 1, G);
    AddEdge(sink - i, sink, 1, G);
  }

  for (j = 1; j <= n; j++){    
    RandomSubset(1, n, d, x);
    for (i = 0; i < d; i++) AddEdge(j, n + x[i], 1, G);
  }
  free(x);
  return G;
}

Graph *SquareMesh(int32_t d, int32_t deg, int32_t r) {
  Graph *G; int32_t i, j, k, source, sink;
  if (d < deg) Barf("Degenerate graph");

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(d*d + 2, sizeof(Edge *));
  G->V = calloc(d*d + 2, sizeof(int32_t));
  InitGraph(G, d*d + 2);  

  for (i = 0; i <= d*d + 1; i++) AddVertex(i, G);
  source = 0; sink = d*d + 1;

  for (i = 1; i <= d; i++){
    AddEdge(source, source + i, 3*r, G);
    AddEdge(sink - i, sink, 3*r, G);
  }

  for (i = 0; i < d - 1; i++)
    for (j = 1; j <= d; j++)
      for (k = 0; k < deg; k++)
        if ((i+1)*d + j + k<= sink - 1)
          AddEdge(i*d + j, (i+1)*d + j + k, RandomInteger(1, r), G);
  return G;
}

Graph *BasicLine(int32_t n, int32_t m, int32_t deg, int32_t range) {
  Graph *G; int32_t i, j, source, sink;
  int32_t *x = calloc(deg, sizeof(int32_t)); // Removed 'const'

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(n*m + 2, sizeof(Edge *));
  G->V = calloc(n*m + 2, sizeof(int32_t));
  InitGraph(G, n*m + 2);  

  for (i = 0; i <= n*m + 1; i++) AddVertex(i, G);
  source = 0; sink = n*m + 1;

  for (i = 1; i <= m; i++){
    AddEdge(source, source + i, deg*range, G);
    AddEdge(sink - i, sink, deg*range, G);
  }

  for (i = source + 1; i < sink; i++){
      RandomSubset(1, m*deg, deg, x);
      for (j = 0; j < deg; j++)
        if (i + x[j] < sink)
          AddEdge(i, i + x[j], RandomInteger(1, range), G);
  }
  free(x);
  return G;
}

Graph *ExponentialLine(int32_t n, int32_t m, int32_t deg, int32_t range) {
  Graph *G; int32_t i, j, source, sink, r;
  int32_t *x = calloc(deg, sizeof(int32_t)); // Removed 'const'

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(n*m + 2, sizeof(Edge *));
  G->V = calloc(n*m + 2, sizeof(int32_t));
  InitGraph(G, n*m + 2);  

  for (i = 0; i <= n*m + 1; i++) AddVertex(i, G);
  source = 0; sink = n*m + 1;

  for (i = 1; i <= m; i++){
    AddEdge(source, source + i, deg*range, G);
    AddEdge(sink - i, sink, deg*range, G);
  }

  for (i = source + 1; i < sink; i++){
      RandomSubset(1, m*deg, deg, x);
      for (j = 0; j < deg; j++){
        r = (x[j] - 1) / m;
        if (i + x[j] < sink)
          AddEdge(i, i + x[j], RandomInteger(1, Range[r]), G);
      }
  }
  free(x);
  return G;
}

Graph *DExponentialLine(int32_t n, int32_t m, int32_t deg, int32_t range) {
  Graph *G; int32_t i, j, source, sink, r;
  int32_t *x = calloc(deg, sizeof(int32_t)); // Removed 'const'

  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(n*m + 2, sizeof(Edge *));
  G->V = calloc(n*m + 2, sizeof(int32_t));
  InitGraph(G, n*m + 2);  

  for (i = 0; i <= n*m + 1; i++) AddVertex(i, G);
  source = 0; sink = n*m + 1;

  for (i = 1; i <= m; i++){
    AddEdge(source, source + i, deg*range, G);
    AddEdge(sink - i, sink, deg*range, G);
  }

  for (i = source + 1; i < sink; i++){
      RandomSubset(-m*deg, m*deg, deg, x);
      for (j = 0; j < deg; j++){
        r = Abs((x[j] - 1) / m);
        if (i + x[j] < sink && i + x[j] > source && x[j] != 0)
          AddEdge(i, i + x[j], RandomInteger(1, Range[r]), G);
      }
  }
  free(x);
  return G;
}

Graph *DinicBadCase(int32_t n) {
  Graph *G; int32_t i;
  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(n, sizeof(Edge *));
  G->V = calloc(n, sizeof(int32_t));
  InitGraph(G, n);  

  for (i = 0; i < n; i++) AddVertex(i, G);
  for (i = 0; i < n-1; i++) AddEdge(i, i+1, n, G);
  for (i = 0; i < n-2; i++) AddEdge(i, n - 1, 1, G);
  return G;
}

Graph *GoldBadCase(int32_t n) {
  Graph *G; int32_t i;
  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(3*n+3, sizeof(Edge *));
  G->V = calloc(3*n+3, sizeof(int32_t));
  InitGraph(G, 3*n+3);  

  for (i = 0; i < 3*n+3; i++) AddVertex(i, G);
  AddEdge(0, 1, n, G);

  for (i = 2; i < n+2; i++){
    AddEdge(1, i, n, G);
    AddEdge(i, i+n, 1, G);
    AddEdge(i+n, 2*n+2, n, G);
  }
  for (i = 2*n+2; i < 3*n+2; i++) AddEdge(i, i+1, n, G);
  return G;
}

Graph *Cheryian(int32_t n, int32_t m, int32_t c, int32_t very_big) {
  Graph *G;
  G = (Graph *) malloc(sizeof(Graph));
  G->A = calloc(4*m*c + 6 + 2*n, sizeof(Edge *));
  G->V = calloc(4*m*c + 6 + 2*n, sizeof(int32_t));
  InitGraph(G, 4*m*c + 6 + 2*n);  

  AddVertex(0, G); AddVertex(1, G); AddVertex(2, G); AddVertex(3, G);

  Gadget(0, 1, n, m, c, G, very_big);
  Gadget(0, 2, n, m, c, G, very_big);
  Gadget(1, 3, n, m, c, G, very_big);
  Gadget(2, 3, n, m, c, G, very_big);

  Bridge(1, 2, n, G);
  Sink(3, G, very_big);
  return G;
}

void Gadget(int32_t a, int32_t b, int32_t n, int32_t m, int32_t c, Graph *G, int32_t very_big) {
  int32_t i, j, v, w;
  v = b;
  for (i = 0; i < m; i++){
    for (j = 0; j < c; j++){
      w = v; v = NewVertex(G); AddEdge(v, w, very_big, G);
    }
    AddEdge(a, v, n, G);
  }
}

void Bridge(int32_t a, int32_t b, int32_t n, Graph *G) {
  int32_t i, v, w,  v1, v2;
  v1 = NewVertex(G); v2 = NewVertex(G);
  AddEdge(a, v1, n, G); AddEdge(v2, b, n, G);
  for (i = 0; i < n; i++){
    v = NewVertex(G); w = NewVertex(G);
    AddEdge(v1, v, n, G); AddEdge(w, v2, n, G); AddEdge(v, w, 1, G);
  }
}

void Sink(int32_t k, Graph *G, int32_t very_big) {
  AddEdge(k, NewVertex(G), very_big, G);
}

int32_t NewVertex(Graph *G) {
  AddVertex(G->size, G);
  return G->size - 1;
}

/* --- Utilities & Manipulation --- */
void InitGraph(Graph *G, int32_t n) {
  for (int32_t i = 0; i < n; i++){
    G->A[i] = (Edge *) 0;
    G->V[i] = FALSE;
  }
  G->size = 0;  G->max_v = -1;
}

void AddVertex(int32_t v, Graph *G) {
  if (G->V[v] == TRUE) Barf("Vertex already present");
  G->V[v] = TRUE;
  G->size++;
  if (v > G->max_v) G->max_v = v;
}

Edge *EdgeLookup(int32_t v1, int32_t v2, Graph *G) {
  Edge *e = G->A[v1];
  while (e != (Edge *) 0){
    if (e->h == v2) return e;
    e = e->next;
  }
  return (Edge *) 0;
}

void AddEdge(int32_t v1, int32_t v2, int32_t a, Graph *G) {
  Edge *e1, *e2;
  if (v1 == v2) Barf("No Loops");

  if ((e1 = EdgeLookup(v1, v2, G)) != (Edge *) 0){
    e1->c += a; return;
  }

  e1 = (Edge *) malloc(sizeof(Edge));
  e2 = (Edge *) malloc(sizeof(Edge));
  e1->mate = e2; e2->mate = e1;
  e1->next = G->A[v1]; G->A[v1] = e1;
  e1->t = v1; e1->h = v2; e1->c = a;
  e2->next = G->A[v2]; G->A[v2] = e2;
  e2->t = v2; e2->h = v1; e2->c = 0;
}

int32_t EdgeCount(Graph *G) {
  int32_t count = 0;
  for (int32_t i = 0; i <= G->max_v; i++){
    if (G->V[i] == FALSE) continue;
    Edge *e = G->A[i];
    while (e != (Edge *) 0){
      if (e->c > 0) count++;
      e = e->next;
    }
  }
  return count;
}

void Barf(char *s) {
  fprintf(stderr, "%s\n", s); exit(-1);
}

int32_t Abs(int32_t x) {
  return (x > 0) ? x : -x;
}

void InitRandom(int32_t seed) {
    struct timeval tp;
    if (seed == 0){
        gettimeofday(&tp, 0);
        srandom(tp.tv_sec + tp.tv_usec);
    } else srandom(seed);
}

int32_t RandomInteger(int32_t low, int32_t high) {
    return random() % (high - low + 1) + low;
}

void RandomSubset(int32_t low, int32_t high, int32_t n, int32_t *x) {
  int32_t i = 0, j, r, flag;
  if (high - low + 1 < n) Barf("Invalid range for Random Subset");
  while (i < n){
    r = RandomInteger(low, high); flag = 0;
    for (j = 0; j < i; j++) if (x[j] == r) flag = 1;
    if (flag == 0) x[i++] = r;
  }
}

void GraphOutput(Graph *G, FILE *f, int32_t s, int32_t t) {
  fprintf(f, "p max %"PRId32" %"PRId32"\n", G->size, EdgeCount(G));
  fprintf(f, "n %"PRId32" s\n", s + 1);
  fprintf(f, "n %"PRId32" t\n", t + 1);
  for (int32_t i = 0; i <= G->max_v; i++) WriteVertex(i, G, f);
}

void WriteVertex(int32_t v, Graph *G, FILE *f) {
  Edge *e = G->A[v];
  while (e != (Edge *) 0){
    if (e->c > 0) fprintf(f, "a %"PRId32" %"PRId32" %"PRId32"\n", e->t + 1, e->h + 1, e->c);
    e = e->next;
  }
}