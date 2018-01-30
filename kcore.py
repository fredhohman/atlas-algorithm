'''
@author : Varun Bezzam for the Polo Club of Data Science
@description : Graph Playground code for graphs represented as edge lists 
@usage : python kcore.py -f <input filename> -n <number of nodes in graph> -e <number of edges in graph> -o <output file name>
'''
import sys

### Reverses graph and appends it to itself
def format_graph(graph):
	reversed_graph = []
	for i in graph:
		numbers = i.split("\t")
		numbers.reverse()
		edge = str(numbers[0]) + "\t" + str(numbers[1])
		reversed_graph.append(edge)
	graph = graph + reversed_graph
	graph = sorted(graph, key = lambda x : int(x.split("\t")[0]))
	return graph

### Finds the start and end indices of a node's neighbors.
def find_start_and_end_indices(graph):
	start = [0]*(NODENUM + 1)
	end = [0]*(NODENUM + 1)
	old = int(graph[0].split("\t")[0])
	start[old] = 0
	for ix, val in enumerate(graph):
		if int(val.split("\t")[0]) != old:
			end[old] = ix - 1
			old = int(val.split("\t")[0])
			start[old] = ix
	end[old] = ix
	return start, end

### Checks if a node is in the graph based on starting and ending indices
def is_node_in_graph(node_start, node_end):
	return not (node_start == 0 and node_end == 0)

### Checks if all edges have been deleted from the graph
def is_graph_empty(edge_labels):
	return all(e != -1 for e in edge_labels)

### Checks if an edge has been deleted from the graph. An edge is considered deleted if it has been labeled.
def is_edge_deleted(edge_label):
	return edge_label != -1

### Finds the degree of all nodes in the graph
def find_degree(start_indices, end_indices, edge_labels):
	degree = []
	for x, y in zip(end_indices, start_indices):
		if not is_node_in_graph(x, y):
			degree.append(0)
		else:
			d = 0
			for i in range(y, x + 1):
				if not is_edge_deleted(edge_labels[i]):
					d += 1
			degree.append(d)
	return degree

### Finds the neighbors for any node in the graph
def neighbors(node, start, end, edge_labels):
	neighbors = []
	if not is_node_in_graph(start[node], end[node]):
		return neighbors
	for i in range(start[node], end[node] + 1):
		if not is_edge_deleted(edge_labels[i]):
			neighbors.append(int(graph[i].split("\t")[1]))
	return neighbors

### Performs k-core decomposition in O(E) time 
### deg, vert, pos are arrays indexed from 1 of size n
### bins starts at index 0
def core(deg, start_indices, end_indices, edge_labels):
	vert = [0] * len(deg)
	pos = [0] * len(deg)
	md = max(deg)
	bins = []
	for d in range(md + 1):
		bins.append(0)
	for v in range(1, NODENUM + 1):
		bins[deg[v]] += 1
	start = 1
	for d in range(md + 1):
		num = bins[d]
		bins[d] = start
		start += num
	for v in range(1, NODENUM + 1):
		pos[v] = bins[deg[v]]
		vert[pos[v]] = v
		bins[deg[v]] += 1
	for d in range(md, 0, -1):
		bins[d] = bins[d-1]
	bins[0] = 1
	for i in range(1, NODENUM + 1):
		v = vert[i]
		for u in neighbors(v, start_indices, end_indices, edge_labels):
			if deg[u] > deg[v]:
				du = deg[u]
				pu = pos[u]
				pw = bins[du]
				w = vert[pw]
				if u != w:
					pos[u] = pw
					pos[w] = pu
					vert[pu] = w
					vert[pw] = u
				bins[du] += 1
				deg[u] -= 1
	return deg

### Labels and deletes edges from the graph
def label_and_delete_edges(graph, is_final_node, edge_labels, peel):
	for ix, edge in enumerate(graph):
		[src, tgt] = [int(x) for x in edge.split("\t")]
		if is_final_node[src] and is_final_node[tgt] and not is_edge_deleted(edge_labels[ix]):
			edge_labels[ix] = peel
	return edge_labels

### Parses command line options
def getopts(argv):
    opts = {}  # Empty dictionary to store key-value pairs.
    while argv:  # While there are arguments left to parse...
        if argv[0][0] == '-':  # Found a "-name value" pair.
            opts[argv[0]] = argv[1]  # Add key and value to the dictionary.
        argv = argv[1:]  # Reduce the argument list by copying it starting from index 1.
    return opts

if __name__ == '__main__':
	myargs = getopts(sys.argv)
	if '-f' in myargs:
		original_graph = open(myargs['-f']).readlines()
	if '-n' in myargs:
		NODENUM = int(myargs['-n'])
	if '-e' in myargs:
		EDGENUM = int(myargs['-e'])
	if '-o' in myargs:
		outputfile = open(myargs['-o'], 'w')
	graph = format_graph(original_graph)
	edge_labels = [-1 for e in graph]
	start_indices, end_indices = find_start_and_end_indices(graph)
	while not is_graph_empty(edge_labels):
		deg = find_degree(start_indices, end_indices, edge_labels)
		cores = core(deg, start_indices, end_indices, edge_labels)
		mc = max(cores)
		is_final_node = [x == mc for x in cores]
		edge_labels = label_and_delete_edges(graph, is_final_node, edge_labels, mc)
	original_labels = []
	for edge in original_graph:
		ix = graph.index(edge)
		original_labels.append(edge_labels[ix])
	for label in original_labels:
		outputfile.write("%s\n" % label)
