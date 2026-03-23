### Adding Static Analysis (GCC `fanalyzer`) 

Static Analyzer looks for logic paths that could lead to crashes before running the code. 

```
g++ -std=c++17 -Wall -Wextra -g -fanalyzer -fsanitize=address tests/test_heap.cpp -o run_tests
```

### Test Steps

#### Compile
`make`

#### Find Optimal k
`python3 benchmark.py --suite A`


* What it does: Generates random graphs, tests $k \in \{2, 3, 4, 5, 8, 16\}$, and calculates the $r$, $i$, $d$, and $u$ ratios, as well as Noshita's expected value.

* Your Action: Open reports/report_operations.csv. Look at the Algo_Time_ms (or total time) column to identify which $k$ value resulted in the fastest execution. Write this number down. You will use it for the rest of the tests.

#### Varying Edges m
`python3 benchmark.py --suite B --k <OPTIMAL_K>`
* What it does: Runs Dijkstra 30 times for each density level and saves the data.

* Your Action: Check reports/report_time_fixed_n.csv. You will use this file later to plot $T/((n+m)\log(n))$ as a function of $m$.

#### Varying Vertices n 
`python3 benchmark.py --suite C --k <OPTIMAL_K>`

* What it does: Runs Dijkstra 30 times for each graph size and saves the data.

* Your Action: Check reports/report_time_fixed_m.csv. You will use this file later to plot $T/((n+m)\log(n))$ as a function of $n$.

* Note: You will combine the data from Suite B and Suite C to apply your linear regression $T(n,m) \sim an^bm^c$.


#### Real World Scaling
`python3 benchmark.py --suite D --files NY.gr EUA.gr --k <OPTIMAL_K>`

* What it does: Runs Dijkstra 30 times from random initial vertices on these massive real-world maps and captures the peak RAM usage using the OS resource monitor.

* Your Action: Open reports/report_dimacs_real.csv. You will use this to report the average time and average memory consumption in your final document.


#### Linear regression
python generate_plots.py --suite R --fileA reports/report_operations.csv --fileB reports/report_time_fixed_n.csv

* This will load both files, apply the logarithmic transformation, solve the multiple linear regression matrix, and print the values for $a$, $b$, and $c$ directly to your terminal so you can copy them into your final report!