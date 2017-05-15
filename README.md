Hungry Birds
============
`Hungry Birds` is a classical example of concurrent producer/consumer problem.

This package implements an unbounded lockless single consumer
multiple producer FIFO queue.


Install
---------
Follwing instructions are for cross platform consideration,

1. Header file `stdatomic.h` is only available for gcc>=4.9 according to [C++11 Support in GCC](https://gcc.gnu.org/projects/cxx-status.html#cxx11), so gcc<=4.8 is not supported.
2. On Mac, replace `#include <malloc.h>` with `#include <sys/malloc.h>`.

Licensing
---------
`Hungry Birds` is freely redistributable under the two-clause BSD License.
Use of this source code is governed by a BSD-style license that can be found
in the `LICENSE` file.
