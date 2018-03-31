# Atlas Edge Decomposition Algorithm

This is the fast, scalable implementation of [edge decomposition based on fixed points of degree peeling][edge-decomp] proposed in [Graph Playgrounds][graph-playground].

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

We first convert a plain text edge list file to a `.bin` to use for algorithm. Input text files should be comma (or tab) separated where each row contains a `source` and `target`. For example:

```
1, 2
1, 3
2, 4
...
```

Convert the text edge list to a `.bin` using the mmap.jar file available here : http://poloclub.gatech.edu/mmap/MMap.zip

```
java -jar mmap.jar Convert <input file>
```

The algorithm takes in this `.bin` file to perform the decomposition. It outputs a `graph-decomposition.csv` file where each row contains three values: the `source`, `target`, and `peel`. The `source` and `target` columns together form the original edge list, and the new column `peel` contains the peel assignment, i.e., what layer an edge belongs to. To run the algorithm, use:
```
./atlas-decomposition <input file>.bin <# of edges> <# of nodes> 
```
It also outputs a `graph-decomposition-info.json` file that contains metadata such as the number of nodes in the graph, number of edges in the graph, time taken to preprocess the data, and time taken to run the algorithm.
## License

MIT License. See [`LICENSE.md`](LICENSE.md).


## Credits

For questions contact [Fred Hohman][fred].

[edge-decomp]: https://link.springer.com/article/10.1007/s13278-014-0191-7
[graph-playground]: http://fredhohman.com
[atlas]: https://github.com/fredhohman/graph-playgrounds
[fred]: http://fredhohman.com
