# Lambding Snakes vs. Coding Monkeys
# ICFPC 2018 solution

Team name is a fusion from 2 teams: "Snakes vs Lambdas" and "Coding Monkeys" of
the corresponding team members that were participating separately before.

## Team members

* Alex
* Den
* Dmitry
* Oleg
* Seva

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

The solver can use multiple threads for processing (-threads #).

The solver output solutions to cppTracesF. We join the current best solutions with candidates using `-mode merge` mode.
One can use `-mode check -levitation 0` for validation of current solutions.

## Assembly

`SolverNonGravitated` and `SolverGravitated`

We call object grounded if it's possible to construct it without high harmonics switched on. The property can be easily checked with BFS.

We run BFS to split whole object by the distance from the floor (one can via faces of the voxels).

Further action is a sheer prescripted work. We construct the object layer by layer.

1. We fission bots into 20 or less if the size is small.
2. These bots can decouple each other and cover one-dimension straps on the current level with `GFill`.
3. Than all bots lift-up on one level and repeat constrution.
4. At the end, we merge them together, and than put a single bot to the origin.

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

We tried incremental reassemble, but the idea did not pay out.

Most of the assembly/disassembly algorithm outputs are preprocessed with an optimizer, which turns on/off levitation, when needed.
