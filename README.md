# Lambding Snakes vs. Coding Monkeys
# ICFPC 2018 solution

Team name is a fusion from 2 teams: "Snakes vs Lambdas" and "Coding Monkeys" of
the corresponding team members that were participating separately before.

## Usage


To run the solution on all problems:
```
cd src/cpp/
make && ./build/cpp_solver
```

For specific problem one can use
```
./build/cpp_solver -test A042
```

The parameter `test` also supports a regexp:
```
./build/cpp_solver -test "D\d\d\d"
```

All the algorithms live in corresponding folders:
```
src/cpp/solver_assembly/
src/cpp/solver_disassembly/
src/cpp/solver_reassembly/
```

Let's take a closer look on them.


## Assembly


## Disassembly

`Solver2D_Demolition`

 * Split the bounding box of figure into long strip of width 30.
 * And then use it as sliding window to swipe the wholer Layer.
 * Continue until reaching ground.


`SolverCubeDemolition`

* This only works on problems less than `30x30x30` dimensions of bounding box.
* It forks bots and puts into corners of the cube.
* Then does one `GVoid` to wipe it
* Doesn't even flip the harmonics



## Reassembly

One of the approaches here is to just use best of Assembly + Disassembly from all of the above solvers.
