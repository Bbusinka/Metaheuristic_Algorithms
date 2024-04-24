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
#include <list>

const std::vector<std::string> readFiles{
    "xqf131" /*, "xqg237", "pma343", "pka379", "bcl380",
    "pbl395", "pbk411", "pbn423", "pbm436", "xql662",
    "xit1083", "icw1483", "djc1785", "dcb2086", "pds2566"*/
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

void TabuSearch(std::vector<size_t> &cycle, const std::vector<Points> &points, double tabuSize,
                double max_iter)
{
    const size_t n{cycle.size()};
    const size_t tabuListSize{static_cast<size_t>(static_cast<double>(n) * tabuSize)};
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

    std::list<std::pair<size_t, size_t>> tabuList;

    int64_t currentCycleChange{0};
    int64_t bestCycleChange{0};
    std::vector<size_t> bestCycle{cycle};

    size_t iterationsWithoutImprovement{0};
    while (iterationsWithoutImprovement < maxIterationsWithoutImprovement)
    {
        int64_t currentBestInversionChange{0};
        std::pair<size_t, size_t> bestInversion;
        for (size_t inversionNum = 0; inversionNum < n; inversionNum++)
        {
            auto inversion = inversions[intDist(randomEngine)];
            if (std::find(tabuList.begin(), tabuList.end(), inversion) != tabuList.end())
            {
                continue;
            }

            size_t indexi = inversion.first;
            size_t indexj = inversion.second;
            if (indexi == 0 && indexj == (n - 1))
            {
                continue;
            }

            size_t prevIndexi = (indexi == 0) ? (n - 1) : indexi - 1;
            size_t nextIndexj = (indexj == (n - 1)) ? 0 : indexj + 1;

            int64_t change{0};
            change -= metric(points[cycle[prevIndexi] - 1], points[cycle[indexi] - 1]);
            change -= metric(points[cycle[indexj] - 1], points[cycle[nextIndexj] - 1]);

            change += metric(points[cycle[indexi] - 1], points[cycle[nextIndexj] - 1]);
            change += metric(points[cycle[prevIndexi] - 1], points[cycle[indexj] - 1]);

            if (change < currentBestInversionChange)
            {
                currentBestInversionChange = change;
                bestInversion = inversion;
            }
        }

        if (currentBestInversionChange < 0)
        {
            std::reverse(cycle.begin() + bestInversion.first, cycle.begin() + bestInversion.second + 1);

            currentCycleChange += currentBestInversionChange;
            if (tabuList.size() == tabuListSize)
            {
                tabuList.pop_front();
            }

            tabuList.push_back(bestInversion);

            if (currentCycleChange < bestCycleChange)
            {
                bestCycleChange = currentCycleChange;
                bestCycle = cycle;
                iterationsWithoutImprovement = 0;
            }
        }
        else
        {
            iterationsWithoutImprovement++;
        }
    }

    cycle = bestCycle;
}

MakeResults
makeTabuSearch(const std::vector<Points> &points, double tabuSize, double max_iter, size_t iterations, std::mt19937 &generator, const Graph_I &graph)
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
        threads.emplace_back(TabuSearch, std::ref(cycles[iter]), std::ref(points),
                             tabuSize, max_iter);
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
    std::cout << "Tabu Search" << std::endl;
    for (const auto &readFile : readFiles)
    {
        std::vector<Points> points;
        const std::string readFileName = readPointsFromFile(readFile, points);

        std::cout << readFileName << std::endl;

        constexpr double tabuSize{0.6};
        constexpr double max_iter{0.3};
        constexpr size_t iter{100};
        const Graph_I graph = initializeGraph(points);

        std::mt19937 randomEngine{std::random_device{}()};

        MakeResults result = makeTabuSearch(points, tabuSize, max_iter, iter, randomEngine, graph);

        std::cout << "Average Cost: " << result.avgWeight << "\nMinimum Cost: " << result.minWeight << "\nBest Solution: ";

        for (size_t indexVertex : result.cycleWithMinWeight)
        {
            std::cout << indexVertex << " ";
        }
    }

    return 0;
}
