import matplotlib.pyplot as plt
import networkx as nx
import math

# Load the data
data = open("1.txt", "r").readlines()

# Split the data into coordinates
coordinates = {}
for line in data:
    x, y = map(int, line.split()[1:])
    coordinates[int(line.split()[0])] = (x, y)

# Create a graph
G = nx.Graph()

# Add the coordinates as nodes to the graph
for i, (x, y) in coordinates.items():
    G.add_node(i, coordinates=(x, y))

# Add edges to the graph with rounded weights specified in the data file
for i in range(1, len(coordinates) + 1):
    for j in range(i + 1, len(coordinates) + 1):
        x1, y1 = coordinates[i]
        x2, y2 = coordinates[j]
        distance = round(math.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2))
        G.add_edge(i, j, weight=distance)

# Find the minimum spanning tree
mst = nx.minimum_spanning_tree(G)

# Calculate the weight of the minimum spanning tree
mst_weight = sum([edge[2]['weight'] for edge in mst.edges(data=True)])

# Check if the minimum spanning tree is a tree
if nx.is_tree(mst):
    # Print the weight of the minimum spanning tree
    print("Weight of the minimum spanning tree:", mst_weight)

# Perform a Depth-First Search (DFS) to get a TSP tour from the MST
def dfs_tsp(graph, start):
    visited = set()
    tour = []
    
    def dfs(node):
        visited.add(node)
        tour.append(node)
        for neighbor in graph.neighbors(node):
            if neighbor not in visited:
                dfs(neighbor)
    
    dfs(start)
    tour.append(start)
    
    return tour

# Set the starting point for the TSP
start = 1    

# Find the TSP tour on the MST
tsp_tour = dfs_tsp(mst, start)

# Calculate the weight of the TSP tour
tsp_weight = sum([G[tsp_tour[i]][tsp_tour[i + 1]]['weight'] for i in range(len(tsp_tour) - 1)])

print("TSP Weight on MST:", tsp_weight)

# Create a figure
fig = plt.figure()

# Plot the minimum spanning tree
pos = nx.get_node_attributes(G, "coordinates")
nx.draw(mst, pos, with_labels=True)

# Draw the TSP tour
tsp_tour_edges = [(tsp_tour[i], tsp_tour[i + 1]) for i in range(len(tsp_tour) - 1)]
tsp_tour_color = 'r'  # You can change the color as desired
nx.draw_networkx_edges(G, pos, edgelist=tsp_tour_edges, edge_color=tsp_tour_color, width=2)

# Show the figure
plt.show()
