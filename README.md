# frequent_pattern_mining
mining frequent itemsets from large data

to compile:

\# g++ simplicial_complex.cpp -pthread

to run:

\# ./a.out configSample_65k 4

result file is "resultFile"

some example results:
(running on Intel(R) Core(TM) i5 CPU M540 @ 2.53GHz | two cores four threads)

\# ./a.out configSample_65k 1

running on 1 thread(s) ...
Initialization: 	1.249 seconds
Building Stacks: 	4.185 seconds
Frequent Patterns: 	10.881 seconds

\# ./a.out configSample_65k 2

running on 2 thread(s) ...
Initialization: 	1.256 seconds
Building Stacks: 	4.217 seconds
Frequent Patterns: 	6.663 seconds

\# ./a.out configSample_65k 4

running on 4 thread(s) ...
Initialization: 	1.258 seconds
Building Stacks: 	4.198 seconds
Frequent Patterns: 	6.050 seconds

