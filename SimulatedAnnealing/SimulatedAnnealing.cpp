#include <iostream>
#include <fstream>
#include <thread>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>
#include <utility>
#include <chrono>
#include <vector>
#include <queue>
#include <stack>

const std::vector<std::string> readFiles{
    "1" /*, "2", "3", "4", "5",
    "6", "7", "8", "9", "10",
    "11", "12", "13", "14", "15"*/
};

struct Points
{
    int64_t x, y;
};

class Graph
{
public:
    struct Edge
    {
        size_t sourceVertex;
        size_t targetVertex;
        int64_t weight;
    };

    class EdgeComparator
    {
    public:
        bool operator()(const Edge &firstEdge, const Edge &secondEdge) { return firstEdge.weight > secondEdge.weight; }
    };

private:
    size_t verticesNum;
    std::vector<std::vector<Edge>> adjacencyList;

public:
    Graph(size_t verticesNumber) : verticesNum{verticesNumber}, adjacencyList(verticesNumber + 1) {}

    void addEdge(size_t firstVertex, size_t secondVertex, int64_t weight)
    {
        adjacencyList[firstVertex].emplace_back(Edge{firstVertex, secondVertex, weight});
        adjacencyList[secondVertex].emplace_back(Edge{secondVertex, firstVertex, weight});
    }

    std::vector<Edge> getEdges() const
    {
        std::vector<Edge> edges{};
        for (size_t vertex = 1; vertex <= verticesNum; vertex++)
        {
            for (const auto &neighbour : adjacencyList[vertex])
            {
                edges.push_back(neighbour);
            }
        }

        return edges;
    }

    size_t getEdgesNumber() const
    {
        size_t edgesNumber{0};
        for (size_t vertex = 1; vertex <= verticesNum; vertex++)
        {
            edgesNumber += adjacencyList[vertex].size();
        }

        return edgesNumber;
    }

    int64_t MST(Graph &mst, size_t startVertex) const
    {
        int64_t totalWeight{0};
        std::vector<bool> visited(verticesNum + 1, false);
        std::vector<int64_t> cost(verticesNum + 1, std::numeric_limits<int64_t>::max());
        std::vector<size_t> parent(verticesNum + 1, 0);
        std::vector<int64_t> weight(verticesNum + 1, 0);

        cost[startVertex] = 0;

        for (size_t i = 0; i < verticesNum; i++)
        {
            size_t minKey{1};
            int64_t minVal{std::numeric_limits<int64_t>::max()};
            for (size_t k = 1; k <= verticesNum; k++)
            {
                if (!visited[k] && cost[k] < minVal)
                {
                    minKey = k;
                    minVal = cost[k];
                }
            }

            visited[minKey] = true;
            totalWeight += minVal;

            for (const auto &neighbour : adjacencyList[minKey])
            {
                if (!visited[neighbour.targetVertex] && neighbour.weight < cost[neighbour.targetVertex])
                {
                    parent[neighbour.targetVertex] = minKey;
                    weight[neighbour.targetVertex] = neighbour.weight;
                    cost[neighbour.targetVertex] = neighbour.weight;
                }
            }
        }

        for (size_t i = 1; i <= verticesNum; i++)
        {
            if (parent[i] != 0)
            {
                mst.addEdge(parent[i], i, weight[i]);
            }
        }

        return totalWeight;
    }

    friend std::ostream &operator<<(std::ostream &os, const Graph &graph)
    {
        for (size_t vertex = 1; vertex <= graph.verticesNum; vertex++)
        {
            for (const auto &neighbour : graph.adjacencyList[vertex])
            {
                if (vertex <= neighbour.targetVertex)
                {
                    os << "(" << vertex << ", " << neighbour.targetVertex << "): " << neighbour.weight << std::endl;
                }
            }
        }

        return os;
    }
};

typedef Graph Graph_I;

struct MakeResults
{
    double minWeight;
    double avgWeight;
    double timePerRepetition;
    std::vector<size_t> cycleWithMinWeight;
};

int64_t metric(const Points &firstPoint, const Points &secondPoint)
{
    return static_cast<int64_t>(sqrt(pow(firstPoint.x - secondPoint.x, 2) + pow(firstPoint.y - secondPoint.y, 2)) +
                                0.5);
}

int64_t cycleWeight(const std::vector<Points> &points, const std::vector<size_t> &cycle)
{
    int64_t cycleWeight{0};

    for (size_t i = 0; i < cycle.size(); i++)
    {
        cycleWeight += metric(points[cycle[i] - 1], points[cycle[(i + 1) % cycle.size()] - 1]);
    }

    return cycleWeight;
}

Graph_I initializeGraph(const std::vector<Points> &points)
{
    Graph_I graph{points.size()};

    for (size_t firstVertex = 0; firstVertex < points.size(); firstVertex++)
    {
        for (size_t secondVertex = firstVertex + 1; secondVertex < points.size(); secondVertex++)
        {
            graph.addEdge(firstVertex + 1, secondVertex + 1, metric(points[firstVertex], points[secondVertex]));
        }
    }

    return graph;
}

std::string readPointsFromFile(const std::string &fileName, std::vector<Points> &points)
{
    std::ifstream inputFile(fileName + ".txt");

    if (!inputFile.is_open())
    {
        std::cerr << "Unable to open file: " << fileName << std::endl;
        return 0;
    }

    int64_t pointIndex, pointX, pointY;
    while (inputFile >> pointIndex >> pointX >> pointY)
    {
        points.push_back({pointX, pointY});
    }

    inputFile.close();

    return fileName;
}

void simulatedAnnealing(std::vector<size_t> &cycle, const std::vector<Points> &points,
                        double t_first, double t_second, double epoki, double max_iter)
{
    const size_t n{cycle.size()};
    double temperature{static_cast<double>(cycleWeight(points, cycle)) * t_first};
    const size_t maxEpochSize{static_cast<size_t>(static_cast<double>(n) * epoki)};
    const size_t maxIterationsWithoutImprovement{static_cast<size_t>(static_cast<double>(n) * max_iter)};

    std::vector<std::pair<size_t, size_t>> inversions;
    for (size_t i = 0; i < n - 1; i++)
    {
        for (size_t j = i + 1; j < n; j++)
        {
            inversions.emplace_back(i, j);
        }
    }

    std::mt19937 randomEngine{std::random_device{}()};
    std::uniform_int_distribution<size_t> intDist(0, inversions.size() - 1);

    int64_t currentCycleChange{0};
    int64_t bestCycleChange{0};
    std::vector<size_t> bestCycle{cycle};

    size_t iterationsWithoutImprovement{0};
    while (iterationsWithoutImprovement < maxIterationsWithoutImprovement)
    {
        iterationsWithoutImprovement++;
        size_t currentIteration{0};
        while (currentIteration < maxEpochSize)
        {
            const auto neighbour = inversions[intDist(randomEngine)];
            size_t indexi = neighbour.first;
            size_t indexj = neighbour.second;

            if (indexi == 0 && indexj == (n - 1))
            {
                continue;
            }

            int64_t change{0};

            size_t prevIndexi = (indexi == 0) ? (n - 1) : indexi - 1;
            size_t nextIndexj = (indexj == (n - 1)) ? 0 : indexj + 1;

            change -= metric(points[cycle[prevIndexi] - 1], points[cycle[indexi] - 1]);
            change -= metric(points[cycle[indexj] - 1], points[cycle[nextIndexj] - 1]);

            change += metric(points[cycle[indexi] - 1], points[cycle[nextIndexj] - 1]);
            change += metric(points[cycle[prevIndexi] - 1], points[cycle[indexj] - 1]);

            if (change < 0)
            {
                std::reverse(cycle.begin() + indexi, cycle.begin() + indexj + 1);
                iterationsWithoutImprovement = 0;
                currentIteration++;
                currentCycleChange += change;

                if (currentCycleChange < bestCycleChange)
                {
                    bestCycleChange = currentCycleChange;
                    bestCycle = cycle;
                }
            }
            else
            {
                std::uniform_real_distribution<double> realDist(0.0, 1.0);
                if (realDist(randomEngine) < std::exp(-change / temperature))
                {
                    std::reverse(cycle.begin() + indexi, cycle.begin() + indexj + 1);
                    currentIteration++;
                    currentCycleChange += change;
                }
            }
        }

        temperature *= t_second;
    }

    cycle = bestCycle;
}

MakeResults
makeSimulatedAnnealing(const std::vector<Points> &points,
                       double t_first, double t_second, double epoki, double max_iter, size_t iterations,
                       std::mt19937 &generator, const Graph_I &graph)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    std::vector<std::vector<size_t>> cycles(iterations, std::vector<size_t>());

    for (size_t iter = 0; iter < iterations; iter++)
    {
        for (size_t vertex = 1; vertex <= points.size(); vertex++)
        {
            cycles[iter].push_back(vertex);
        }
        std::shuffle(cycles[iter].begin(), cycles[iter].end(), generator);
    }

    for (size_t iter = 0; iter < iterations; iter++)
    {
        threads.emplace_back(simulatedAnnealing, std::ref(cycles[iter]), std::ref(points),
                             t_first, t_second, epoki, max_iter);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::vector<double> results(iterations);
    for (size_t iter = 0; iter < iterations; iter++)
    {
        results[iter] = static_cast<double>(cycleWeight(points, cycles[iter]));
    }

    std::chrono::duration<double> elapsed = end - start;

    double minWeight{std::numeric_limits<double>::max()};
    size_t minWeightIndex{0};
    double weightsSum{0.0};
    for (size_t iter = 0; iter < iterations; iter++)
    {
        if (results[iter] < minWeight)
        {
            minWeight = results[iter];
            minWeightIndex = iter;
        }
        weightsSum += static_cast<double>(results[iter]);
    }

    return {
        minWeight,
        weightsSum / static_cast<double>(iterations),
        elapsed.count() / static_cast<double>(iterations),
        cycles[minWeightIndex]};
}

int main()
{
    std::cout << "Symulowane wyzarzanie" << std::endl;
    for (const auto &readFile : readFiles)
    {
        std::vector<Points> points;
        const std::string readFileName = readPointsFromFile(readFile, points);

        std::cout << readFileName << std::endl;

        constexpr double t_first{0.5};
        constexpr double t_second{0.95};
        constexpr double epoki{0.2};
        constexpr double max_iter{0.1};
        constexpr size_t iterations{100};
        const Graph_I graph = initializeGraph(points);

        std::mt19937 randomEngine{std::random_device{}()};

        MakeResults result = makeSimulatedAnnealing(points, t_first, t_second, epoki, max_iter, iterations, randomEngine, graph);

        std::cout << "Average Cost: " << result.avgWeight << "\nMinimum Cost: " << result.minWeight << "\nBest Solution: ";

        for (size_t indexVertex : result.cycleWithMinWeight)
        {
            std::cout << indexVertex << " ";
        }
    }

    return 0;
}
