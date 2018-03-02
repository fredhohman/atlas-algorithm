# Graph Playground Algorithm

This is the fast, scalable implementation of [edge decomposition based on fixed points of degree peeling][edge-decomp] proposed in [Graph Playgrounds][graph-playground].

## Installation
Download or clone this repository.
From the directory, compile the code by 

```
compile
```

## Usage
We first convert a plain text edge list file to a `.bin` to use for algorithm. Input text files should be comma (or tab) separated where each row contains a `source` and `target`. For example:

```
1, 2
1, 3
2, 4
...
```

Convert the text edge list to a `.bin` using

```
text to bin
```

The algorithm takes in this `.bin` file to perform the decomposition. It outputs a `.csv` file where each row contains three values: the `source`, `target`, and `peel`. The `source` and `target` columns together form the original edge list, and the new column `peel` contains the peel assignment, i.e., what layer an edge belongs to.

## Citation
Graph Playgrounds  
Fred Hohman, Varun Bezzam, James Abello, Duen Horng Chau.

## License
MIT License. See `LICENSE.md`.

## Credits
For questions contact [Fred Hohman][fred].

[edge-decomp]: https://link.springer.com/article/10.1007/s13278-014-0191-7
[graph-playground]: http://fredhohman.com
[fred]: http://fredhohman.com
