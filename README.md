# Lambding Snakes vs. Coding Monkeys ICFPC 2018 solution



### Assembling grounded object

We call object grounded if it's possible to construct it without high harmonics switched on. The property can be easily checked with BFS.

We run BFS to split whole object by the distance from the floor (one can via faces of the voxels).

Further action is a sheer prescripted work. We construct the object layer by layer.

1. We fission bots into 20 or less if the size is small.
2. These bots can decouple each other and cover one-dimension straps on the current level with GFill.
3. Than all bots lift-up on one level and repeat constrution.
4. At the end, we merge them together, and than put a single bot to the origin.
