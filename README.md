# Atlas Edge Decomposition Algorithm

This is the fast, scalable implementation of [edge decomposition based on fixed points of degree peeling][edge-decomp] used in [Atlas][atlas].

For the main Atlas visualization repository, see [github.com/fredhohman/atlas][atlas].


## Installation

Download or clone this repository.

```bash
git clone https://github.com/fredhohman/atlas-algorithm.git
```

From the directory, compile the code by 

```
 g++ -o atlas-decomposition -g -fopenmp -O3 parallelkcore.cpp
```


## Usage

We first convert a plain text edge list file to a `.bin` to use for algorithm. Input text files should be comma (or tab) separated where each row contains a `source` and `target` (these must be numbers).

**Note:** graphs should not have:

* self-loops / self-edges
* duplicate edges (multi-graph)

For example, a graph with three edges would look like:

```
1, 2
1, 3
2, 4
```

Convert the text edge list to a `.bin` using the mmap.jar file available here: http://poloclub.gatech.edu/mmap/MMap.zip

```bash
java -jar mmap.jar Convert <myGraph>
```

The algorithm takes in this `.bin` file to perform the decomposition. It outputs a `myGraph-decomposition.csv` file where each row contains three values: the `source`, `target`, and `peel`. The `source` and `target` columns together form the original edge list, and the new column `peel` contains the peel assignment, i.e., what layer an edge belongs to. To run the algorithm, use:

```bash
./atlas-decomposition <myGraph>.bin <# of edges> <# of vertices> 
```

It also outputs a `myGraph-decomposition-info.json` file that contains metadata such as the number of vertices in the graph, number of edges in the graph, time taken to preprocess the data, and time taken to run the algorithm.


## Example

For an example of what the output looks like, see [github.com/fredhohman/atlas/data][example]


## License

MIT License. See [`LICENSE.md`](LICENSE.md).


## Contact

For questions or support [open an issue][issues] or contact [Fred Hohman][fred].

[edge-decomp]: https://link.springer.com/article/10.1007/s13278-014-0191-7
[atlas]: https://github.com/fredhohman/atlas
[fred]: http://fredhohman.com
[example]: https://github.com/fredhohman/atlas/tree/master/data
[issues]: https://github.com/fredhohman/atlas-algorithm/issues