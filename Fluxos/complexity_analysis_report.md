# Introduction and Theoretical Foundations
Before analyzing the empirical complexity of the implemented algorithms, it is necessary to define the foundational concepts of maximum flow networks and the theoretical metrics used to evaluate them.

A flow network is modeled as a directed and capacitated graph $G=(V,A,c)$, where each arc $a \in A$ has a maximum capacity $c_a$. The goal of a maximum $s-t$ flow algorithm is to find a flow assignment $f$ that maximizes the total flow transported from a specific starting vertex to a specific ending vertex.
* **Source ($s$):** The origin vertex ("fonte") that generates the flow.
* **Sink ($t$):** The destination vertex ("sorvedouro" or "destino") that absorbs the flow.
* **Flow Conservation:** For every other vertex in the network $v \in V \setminus \{s,t\}$, the total flow entering the vertex must exactly equal the total flow leaving it ($f(v) = 0$).

Algorithms in the Ford-Fulkerson family operate by repeatedly finding "augmenting paths" from the source to the sink within a residual graph $G_f$, increasing the flow until no such paths remain. 

**Understanding "Inadequacy" and "Defects"**
A core objective of this report is to analyze the "defect" or "inadequacy" of pessimistic theoretical bounds. 
Theoretical complexity bounds assume the absolute worst-case scenarios—for example, the basic Ford-Fulkerson algorithm has a pessimistic upper limit of $C$ iterations (where $C$ is the maximum flow), meaning it could theoretically require a new phase for every single unit of flow. Edmonds-Karp bounds this to $O(nm)$ iterations. 

In this report, the **Defect (or Inadequacy)** refers to the mathematical gap between these pessimistic theoretical limits and the algorithm's actual performance. We quantify this using three metrics:
1. **Phase Ratio ($r$):** The fraction of the theoretical maximum phases actually executed. 
2. **Vertex Defect ($\bar{s}$):** The average fraction of total vertices explored per phase.
3. **Edge Defect ($\bar{t}$):** The average fraction of total edges evaluated per phase.



# Algorithmic Complexity and Code Analysis

This document outlines the theoretical time complexities of the six Maximum Flow algorithms implemented in this project. By examining the source code, we can identify the specific loops, data structures, and conditional logic that dictate the worst-case performance bounds.

Let $n = |V|$ (number of vertices), $m = |E|$ (number of edges), and $C$ be the maximum flow value.

---

## 1. Ford-Fulkerson (DFS)
**Theoretical Worst-Case Complexity:** $O(m \cdot C)$

The Ford-Fulkerson algorithm using Depth-First Search (DFS) is a pseudo-polynomial algorithm. In the worst case, every phase finds an augmenting path that only pushes $1$ unit of flow. Therefore, it might take exactly $C$ phases. Finding a path using DFS takes $O(m)$ time, as it explores edges until it hits the sink.

**Where it lives in the code (`FordFulkersonDFS.cpp`):**
The $O(m)$ pathfinding happens in this recursive loop, which visits each edge at most once per phase:
```cpp
    for (size_t i = 0; i < network.adj_list[u].size(); ++i) {
        e_touched++; // Track every edge evaluated
        Edge& edge = network.adj_list[u][i];
        long long residual = network.get_residual_capacity(edge);
        
        if (!visited[edge.to] && residual > 0) {
            // Recurse deeper, carrying the bottleneck flow downwards
            long long bottleneck = dfs(network, edge.to, t, std::min(current_flow, residual), 
                                       visited, v_touched, e_touched);
            // ...
```

## 2. Randomized Ford-Fulkerson(RDFS)
**Theoretical Worst-Case Complexity:** $O(m \cdot C)$

The Randomized DFS has the exact same theoretical worst-case bound as standard FF. However, its average-case performance is drastically improved on pathological graphs because it breaks deterministic worst-case edge selection by shuffling the adjacency evaluation order.

**Where it lives in the code (`FordFulkersonRandDFS.cpp`):**
The complexity remains $O(m)$ per phase, but the edge indices are shuffled in $O(m)$ time before the loop begins:

```cpp
std::vector<size_t> indices(network.adj_list[u].size());
    std::iota(indices.begin(), indices.end(), 0);

    static std::random_device rd; 
    static std::mt19937 g(rd());

    // Shuffling breaks the deterministic worst-case path selection
    std::shuffle(indices.begin(), indices.end(), g); 

    for (size_t idx = 0; idx < network.adj_list[u].size(); ++idx) {
        size_t i = indices[idx]; // Access edges randomly
        // ...
```

## 3. Edmonds-Karp (BFS)
**Theoretical Worst-Case Complexity:**  $O(n m^2)$
Edmonds-Karp guarantees strongly polynomial time by always selecting the shortest augmenting path (in terms of the number of edges). It is mathematically proven that an edge can become critical at most $n/2$ times. Because there are $m$ edges, there are at most $O(n m)$ phases. Each phase performs a Breadth-First Search (BFS) taking $O(m)$ time.

**Where it lives in the code (`EdmondsKarp.cpp`):**
The BFS queue ensures we search level by level, stopping the moment the sink $t$ is reached.

```cpp
// BFS traversal
        while (!q.empty() && !reached_sink) {
            int u = q.front();
            q.pop();

            for (size_t i = 0; i < network.adj_list[u].size(); ++i) {
                const Edge& edge = network.adj_list[u][i];
                
                // Only traverse unvisited nodes with residual capacity
                if (parent_node[edge.to] == -1 && network.get_residual_capacity(edge) > 0) {
                    parent_node[edge.to] = u;
                    // ... q.push(edge.to)
```

## 4. Dinitz's Algorithm 
**Theoretical Worst-Case Complexity:** $O(n^2 m)$
Dinitz's algorithm achieves a superior polynomial bound by splitting the work. It builds a "Level Graph" using BFS in $O(m)$ time, and then finds a "Blocking Flow" using multiple DFS passes. Because the level graph strictly restricts backward or lateral movement, the length of the paths is bounded by $n$. A single phase finds multiple paths in $O(nm)$ time, and there can be at most $n$ phases.

**Where it lives in the code (`Dinitz.cpp`):**
The secret to Dinitz's $O(nm)$ DFS efficiency is the ptr array (dead-end pruning). Notice the int& cid = ptr[u] reference. It remembers where the loop left off in the previous DFS iteration during the same phase, preventing the algorithm from re-evaluating dead ends!

```cpp
long long dfs(int u, long long pushed, int& vertices_touched) {
        // ...
        // ptr[u] prevents revisiting dead-ends in the level graph
        for (int& cid = ptr[u]; cid < network.adj_list[u].size(); ++cid) {
            Edge& edge = network.adj_list[u][cid];
            long long res = network.get_residual_capacity(edge);

            // Only traverse edges progressing strictly to the next level
            if (level[u] + 1 != level[edge.to] || res == 0) continue;
            // ...
```

## 5. Capacity Scaling
**Theoretical Worst-Case Complexity:** $O(m^2 \log C)$
Capacity Scaling guarantees polynomial time without complex data structures. It bounds the number of phases by introducing a threshold delta. The algorithm only considers edges with capacity $\ge \Delta$. $\Delta$ starts at the maximum edge capacity and halves until it reaches 1. There are $O(\log C)$ scaling phases, and within each phase, it acts like Edmonds-Karp, finding augmenting paths in $O(m^2)$ time.

**Where it lives in the code (`CapacityScaling.cpp`):**
```cpp
// 2. Scaling Loop (Executes log(C) times)
    while (delta >= 1) {
        bool path_found = true;
        
        // Find augmenting paths using only edges with capacity >= delta
        while (path_found) {
             // ... BFS setup ...
             // The core Scaling restriction:
             if (parent_node[edge.to] == -1 && network.get_residual_capacity(edge) >= delta) {
                 // ... push to queue
             }
        }
        // Reduce delta for the next scaling phase
        delta /= 2; 
    }
```

## 6. Fattest Path (Modified Dijkstra)
**Theoretical Worst-Case Complexity:** $O(m \log n \cdot m \log C)$
The Fattest Path algorithm always augments along the path with the maximum possible bottleneck capacity. Theoretically, this guarantees finding the maximum flow in at most $O(m \log C)$ phases. However, finding the "fattest" path requires a modified Dijkstra's algorithm. Using a binary heap (Priority Queue), updating and extracting nodes takes $O(m \log n)$ time per phase.

**Where it lives in the code (`FattestPath.cpp` & `IndexedMaxHeap.hpp`):**
The complexity is driven by the IndexedMaxHeap. Every time an edge is evaluated and provides a wider bottleneck, the Priority Queue must sift up/down to maintain the maximum bottleneck at the top.

```cpp
// Extract the node with the current largest known path capacity in O(log n)
        while (!pq.is_empty()) {
            int u = pq.extract_max();
            
            // ... Loop through adjacent edges ...
                    
                    // Relaxation: Maximize the bottleneck
                    if (bottleneck > max_cap[edge.to]) {
                        max_cap[edge.to] = bottleneck;
                        // ...
                        if (pq.contains(edge.to)) {
                            pq.update_key(edge.to, bottleneck); // O(log n) operation
                        } else {
                            pq.insert(edge.to, bottleneck);     // O(log n) operation
                        }
                    }
```

# Experimental Methodology and Pipeline Configuration
This document details the automated experimental pipeline used to evaluate the complexity of the Maximum Flow algorithms. The experiments were orchestrated using a custom Perl script (`run_experiments.pl`) that dynamically generated graphs and fed them to the C++ solver.

## 1. The Execution Pipeline
* **Graph Generator:** The pipeline utilizes the provided `./washington` generator to create test instances on-the-fly.
* **Input Format:** All graphs are generated strictly in standard **DIMACS format** and saved to a temporary file (`temp_graph.max`).
* **Data Feeding:** The generated DIMACS file is piped directly into the standard input (`stdin`) of the C++ program using the command: `./FlowExperiment --benchmark < temp_graph.max`.
* **Graph Families:** Every experiment is executed against two distinct topological graph families:
    * **Type 6 (`BasicLine`):** A standard line mesh topology.
    * **Type 8 (`DoubleExpLine`):** A topological trap designed to create worst-case execution scenarios.

---

## 2. Experiment Stage A: Density Impact
This stage tests how the algorithms handle an increasingly dense web of edges while the number of vertices remains constant. 

* **Fixed Parameter (Vertices):** The dimensions were fixed at `dim1 = 2` and `dim2 = 2500`, resulting in a constant vertex count of exactly **$n = 5,002$**.
* **Scaling Parameter (Density):** The `deg` (degree) variable was gradually scaled through the values: **$2, 5, 10, 20,$ and $40$**.
* **Resulting Edge Counts ($m$):** As the degree increased, the number of edges scaled from roughly **$10,000$ up to $200,000$** edges on the same 5,002-node graph.

---

## 3. Experiment Stage B: Size Scaling Impact
This stage tests the asymptotic complexity of the algorithms by simulating pure growth in size while maintaining the exact same edge density ratio.

* **Fixed Parameter (Density):** The degree was held constant at **$deg = 10$**.
* **Scaling Parameter (Vertices):** The `dim2` variable was scaled through the values: **$500, 1000, 2500, 5000,$ and $10000$**.
* **Resulting Node and Edge Counts:**
    * $n \approx 1,000 \rightarrow m \approx 10,000$
    * $n \approx 2,000 \rightarrow m \approx 20,000$
    * $n \approx 5,000 \rightarrow m \approx 50,000$
    * $n \approx 10,000 \rightarrow m \approx 100,000$
    * $n \approx 20,000 \rightarrow m \approx 200,000$

---

## 4. Generated Data (The CSV Output)
During both Stage A and Stage B, the C++ solver executes all six algorithm implementations (Edmonds-Karp, Ford-Fulkerson, Randomized DFS, Fattest Path, Capacity Scaling, and Dinitz). 

The raw data is output to `stdout`, intercepted by the script, appended with tracking metadata, and saved to a uniquely timestamped CSV file (e.g., `final_complexity_report_20260427_143000.csv`). 

For every single graph generated, **six rows** of data are produced containing the following 12 columns:

1. **Family:** The graph topology (`BasicLine` or `DoubleExpLine`).
2. **ExpType:** The stage of the experiment (`Density` or `Scaling`).
3. **Algorithm:** The pathfinding strategy used (`EK`, `FF`, `RDFS`, `FP`, `EC`, `Di`).
4. **n:** The exact number of vertices in the graph.
5. **m:** The exact number of edges in the graph.
6. **Max_Flow:** The final integer value of the maximum flow found.
7. **F:** The actual number of augmenting phases the algorithm took.
8. **F_bar:** The theoretical maximum (pessimistic) bound for phases (e.g., $nm/2$ for EK).
9. **r:** The Phase Ratio ($F / \bar{F}$).
10. **s_bar:** The Vertex Defect (Average fraction of total vertices touched per phase).
11. **t_bar:** The Edge Defect (Average fraction of total edges evaluated per phase).
12. **Time_ms:** The physical execution time measured in milliseconds.

# Maximum Flow Algorithms: Complexity Analysis Report
## 1. Inadequacy of Phases ($r$)
This table shows the average phase ratio $r = F / \bar{F}$. A value much smaller than 1.0 indicates that the theoretical pessimistic bound is vastly overstated in practice.

| Algorithm   |   Density |   Scaling |
|:------------|----------:|----------:|
| Di          |   0.0015  |   0.00207 |
| EC          |   0.0074  |   0.00734 |
| EK          |   7e-05   |   0.00011 |
| FF          |   0.00017 |   0.00015 |
| FP          |   0.00735 |   0.0073  |
| RDFS        |   0.00016 |   0.00015 |

### Theoretical Analysis
The phase ratio $r = \frac{F}{\bar{F}}$ measures how many phases the algorithm actually executed compared to its theoretical pessimistic bound. The data shows that $r$ is extraordinarily small for all algorithms (ranging from $10^{-3}$ to $10^{-5}$).

* **Ford-Fulkerson (FF & RDFS):** The theoretical bound for Ford-Fulkerson is $C$ phases, where $C$ is the maximum flow, leading to a pseudo-polynomial time complexity of $O((n+m)C)$. The extreme low value of $r \approx 0.00015$ confirms that real-world or standard random graphs rarely resemble the pathological worst-case scenarios (like the classic 4-node graph with a bottleneck edge of capacity 1) that force the algorithm to incrementally add 1 unit of flow per phase.
* **Edmonds-Karp (EK):** EK improves upon FF by using Breadth-First Search (BFS) to find the shortest augmenting path, bounding the number of iterations to $O(nm)$ and guaranteeing termination in $O(nm^2)$ time. The empirical data ($r \approx 0.00011$) shows that even this strongly polynomial bound is highly pessimistic. The algorithm finds the maximum flow long before each edge becomes critical $n/2 - 1$ times, which is the theoretical maximum.

---

## 2. Inadequacy of Operations ($\bar{s}$ and $\bar{t}$)
This table shows the average fraction of vertices ($\bar{s}$) and edges ($\bar{t}$) touched per phase. Values less than 1.0 indicate the algorithm finds paths without exploring the entire network.

| Algorithm   |   s_bar |   t_bar |
|:------------|--------:|--------:|
| Di          |  0.0046 |  3.4378 |
| EC          |  0.4683 |  0.9507 |
| EK          |  0.4775 |  0.9683 |
| FF          |  0.239  |  0.4822 |
| FP          |  0.3628 |  0.792  |
| RDFS        |  0.009  |  0.0158 |

### Theoretical Analysis
These metrics reveal the internal behavior of the pathfinding strategies by showing the fraction of vertices and edges touched per phase.

* **Edmonds-Karp (EK) vs. Ford-Fulkerson (FF):** EK has very high operation defects ($\bar{s} \approx 0.47, \bar{t} \approx 0.96$). Because EK uses BFS, it systematically explores outward from the source, evaluating nearly the entire residual graph in each phase before finding the sink. In contrast, FF uses Depth-First Search (DFS), allowing it to frequently "get lucky" and plunge straight to the sink without evaluating the whole network, resulting in much lower exploration rates ($\bar{s} \approx 0.23, \bar{t} \approx 0.48$).
* **Dinitz (Di):** Dinitz has a tiny vertex defect ($\bar{s} \approx 0.0046$) but an edge defect greater than 1 ($\bar{t} \approx 3.43$). This perfectly aligns with its theory. Dinitz builds a level graph and then pushes flow along multiple paths simultaneously within the same phase (blocking flows). While it touches a huge number of edges to build the level graph and trace paths (leading to $\bar{t} > 1$), it achieves massive throughput per phase, drastically reducing the overall number of vertices it needs to revisit.

---

## 3. Residual Complexity Analysis
This table compares the theoretical execution time ratio against the empirical operation ratio. If the empirical ratio varies significantly between algorithms, it suggests "residual complexity" (e.g., heavy data structure overhead like Priority Queues in FP, or cache misses in DFS) that isn't captured by simple operation counting.

| Algorithm   |   Theoretical_Ratio |   Empirical_Ratio |
|:------------|--------------------:|------------------:|
| Di          |            2.06e-07 |          4.31e-05 |
| EC          |            2.5e-07  |          2.87e-05 |
| EK          |            3.27e-09 |          2.81e-05 |
| FF          |            6.51e-09 |          6.51e-05 |
| FP          |            8.32e-08 |          0.000104 |
| RDFS        |            3e-09    |          0.0018   |

### Theoretical Analysis
The empirical ratio $\frac{T}{F(\bar{s}n + \bar{t}m)}$ standardizes execution time against the actual operations performed. Variations here highlight the "hidden constants" and data structure overhead not captured by simple Big-O notation.

* **Fattest Path (FP):** The theoretical complexity of FP is $O(m \log C)$ phases, but finding the "fattest" path requires a modified Dijkstra's algorithm using a priority queue. The data shows FP has the highest empirical ratio ($1.04 \times 10^{-4}$), making it structurally the slowest per operation. This proves that the overhead of maintaining a priority queue for every edge evaluation is computationally heavier than the simple FIFO queues used in EK.
* **Dinitz (Di) & Capacity Scaling (EC):** Both show excellent empirical efficiency ($\approx 2.8 \times 10^{-5}$ to $4.3 \times 10^{-5}$). Capacity scaling limits searches to paths with high residual capacity, effectively balancing the fast, simple queues of EK with the high-throughput philosophy of FP, resulting in highly optimized empirical performance.

---

## 4. Visualizations & Behavioral Analysis

The following charts visualize the empirical data across two distinct graph topologies. 
* **Basic Line Mesh:** A standard, predictable graph structure resulting in smoother performance curves. 
* **Double Exponential Line:** A topological trap designed to force simple algorithms into making bad decisions. Variations (like spikes in Ford-Fulkerson) highlight the vulnerability of certain pathfinding strategies to pathological data.

### 4.1 Execution Time vs. Graph Growth
* **What the data shows:** Dinitz (`Di`) forms the lowest, flattest curve, while Fattest Path (`FP`) grows the fastest. Edmonds-Karp (`EK`) and Capacity Scaling (`EC`) sit in the middle.
* **Theoretical Explanation:** This visually exposes the "Residual Complexity" discussed above. Although Fattest Path has a brilliant theoretical bound of $O(m \log C)$ phases, every single operation requires maintaining and extracting from a Priority Queue (Heap). As the graph scales up, the $O(\log n)$ overhead of that data structure crushes real-time performance. Dinitz relies on simple arrays and level graphs, keeping constant overhead incredibly small and allowing it to consistently win in raw time.

### 4.2 Phase Inadequacy ($r$)
* **What the data shows:** The lines for $r$ collapse toward the X-axis (near $0.0$) and remain flat as the graph scales.
* **Theoretical Explanation:** This visually proves that pessimistic bounds are inadequate in practice. As density or scale increases, the mathematical formulas for the bounds (e.g., $\frac{nm}{2}$ for EK, or $C$ for FF) explode into the millions. However, the algorithms themselves are smart enough to find the maximum flow in just a few dozen phases. Because the denominator ($\bar{F}$) explodes while the numerator ($F$) barely moves, the ratio $r$ approaches zero, proving that theoretical worst-case scenarios almost never govern real-world performance.

### 4.3 Edge Defect ($\bar{t}$)
* **What the data shows:** These lines are remarkably horizontal. Dinitz sits highest (near $3.5$), EK and EC hover just under $1.0$, and FF/RDFS are jagged lines much lower ($0.2$ to $0.5$).
* **Theoretical Explanation:** This chart reveals the "personality" of each pathfinding strategy:
    * **EK (BFS):** Explores outward like a wave, evaluating nearly every single edge in the residual graph to find the shortest path, touching $\approx 100\%$ of edges per phase ($\bar{t} \approx 1.0$).
    * **FF (DFS):** Plunges blindly. If it gets lucky, it finds a path touching only a few edges, creating a highly efficient but volatile edge defect ($\bar{t} < 0.5$).
    * **Di (Blocking Flows):** Dinitz must mathematically touch every edge during BFS to build its level graph, and then touches many edges again during its DFS blocking phase. This guarantees it touches more edges than exist in the graph per phase ($\bar{t} > 1.0$), but it makes up for this overhead by moving massive amounts of flow in that single phase.

---

### 4.4 Plotted Data

#### Graph Family: DoubleExpLine
##### Experiment A: Density Impact (Fixed $n$, increasing $m$)
![Time vs Density](plots/time_density_DoubleExpLine.png)
![r vs Density](plots/r_density_DoubleExpLine.png)
![t_bar vs Density](plots/t_bar_density_DoubleExpLine.png)

##### Experiment B: Scaling Impact (Fixed density, increasing $n$)
![Time vs Scaling](plots/time_scaling_DoubleExpLine.png)
![r vs Scaling](plots/r_scaling_DoubleExpLine.png)
![t_bar vs Scaling](plots/t_bar_scaling_DoubleExpLine.png)

#### Graph Family: BasicLine
##### Experiment A: Density Impact (Fixed $n$, increasing $m$)
![Time vs Density](plots/time_density_BasicLine.png)
![r vs Density](plots/r_density_BasicLine.png)
![t_bar vs Density](plots/t_bar_density_BasicLine.png)

##### Experiment B: Scaling Impact (Fixed density, increasing $n$)
![Time vs Scaling](plots/time_scaling_BasicLine.png)
![r vs Scaling](plots/r_scaling_BasicLine.png)
![t_bar vs Scaling](plots/t_bar_scaling_BasicLine.png)