### Adding Static Analysis (GCC `fanalyzer`) 

Static Analyzer looks for logic paths that could lead to crashes before running the code. 

```
g++ -std=c++17 -Wall -Wextra -g -fanalyzer -fsanitize=address tests/test_heap.cpp -o run_tests
```

### Test Steps

#### Compile
`make`

