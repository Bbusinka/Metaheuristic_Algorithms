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
#include <numeric>
#include <map>

const std::vector<std::string> readFiles{
    "xqf131" /*, "xqg237" , "pma343", "pka379", "bcl380",
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
                auto &neighbours = adjacencyList[vertex];
                edges.insert(edges.end(), neighbours.begin(), neighbours.end());
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
        const Points &point1 = points[cycle[i] - 1];
        const Points &point2 = points[cycle[(i + 1) % cycle.size()] - 1];
        cycleWeight += metric(point1, point2);
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

size_t localSearch(std::vector<size_t> &cycle, const std::vector<Points> &points)
{
    const size_t n{cycle.size()};
    size_t improvements{0};

    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<size_t> dist(0, n - 1);

    bool foundNewCandidate{true};
    while (foundNewCandidate)
    {
        foundNewCandidate = false;
        int_fast32_t newCandidateWeightChange{0};
        size_t besti, bestj;

        for (size_t invNum = 0; invNum < n; invNum++)
        {
            size_t indexi = dist(gen);
            size_t indexj = dist(gen);

            if (indexi == 0 && indexj == (n - 1))
            {
                continue;
            }
            else if (indexi == indexj)
            {
                continue;
            }
            else if (indexi > indexj)
            {
                const size_t tmp{indexi};
                indexi = indexj;
                indexj = tmp;
            }

            int_fast32_t change{0};

            size_t prevIndexi = (indexi == 0) ? (n - 1) : indexi - 1;
            size_t nextIndexj = (indexj == (n - 1)) ? 0 : indexj + 1;

            change -= metric(points[cycle[prevIndexi] - 1], points[cycle[indexi] - 1]);
            change -= metric(points[cycle[indexj] - 1], points[cycle[nextIndexj] - 1]);

            change += metric(points[cycle[indexi] - 1], points[cycle[nextIndexj] - 1]);
            change += metric(points[cycle[prevIndexi] - 1], points[cycle[indexj] - 1]);

            if (change < newCandidateWeightChange)
            {
                newCandidateWeightChange = change;
                foundNewCandidate = true;
                besti = indexi;
                bestj = indexj;
            }
        }

        if (foundNewCandidate)
        {
            std::reverse(cycle.begin() + besti, cycle.begin() + bestj + 1);
            improvements++;
        }
    }

    return improvements;
}

typedef std::pair<std::vector<size_t>, int_fast32_t> Individual;

std::pair<Individual, Individual> ox(const Individual &parent1, const Individual &parent2, std::mt19937 &gen)
{
    std::uniform_int_distribution<size_t> dist(1, parent1.first.size() - 2);
    size_t start = dist(gen);
    size_t end = dist(gen);
    while (start == end)
    {
        end = dist(gen);
    }

    if (start > end)
    {
        std::swap(start, end);
    }

    std::vector<size_t> child1(parent1.first.size(), 0);
    std::vector<size_t> child2(parent1.first.size(), 0);

    std::map<size_t, bool> mapping1{};
    std::map<size_t, bool> mapping2{};
    for (size_t i = start; i <= end; i++)
    {
        child1[i] = parent1.first[i];
        child2[i] = parent2.first[i];

        mapping1[parent1.first[i]] = true;
        mapping2[parent2.first[i]] = true;
    }

    std::vector<size_t> missing1{};
    std::vector<size_t> missing2{};

    for (size_t i = 0; i < parent1.first.size(); i++)
    {
        size_t index = (end + i + 1) % parent1.first.size();
        if (mapping1.find(parent2.first[index]) == mapping1.end())
        {
            missing1.push_back(parent2.first[index]);
        }

        if (mapping2.find(parent1.first[index]) == mapping2.end())
        {
            missing2.push_back(parent1.first[index]);
        }
    }

    for (size_t i = 0; i < missing1.size(); i++)
    {
        size_t index = (end + i + 1) % parent1.first.size();
        child1[index] = missing1[i];
        child2[index] = missing2[i];
    }

    return std::make_pair(Individual{child1, 0}, Individual{child2, 0});
}

MakeResults genetic(std::vector<Individual> &population, const std::vector<Points> &points,
                    size_t iterationsNumber, float crossProbability, float mutationProbability, std::mt19937 &gen)
{
    double totalCost = 0.0;

    for (size_t iteration = 0; iteration < iterationsNumber; iteration++)
    {
        std::sort(population.begin(), population.end(),
                  [](const Individual &a, const Individual &b)
                  { return a.second < b.second; });

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        // crossing - choosing parents
        std::vector<Individual> parents{};
        size_t parentsNumber{population.size() / 2};
        while (parents.size() < parentsNumber)
        {
            for (size_t parent = 0; parent < population.size(); parent++)
            {
                if (parents.size() >= parentsNumber)
                {
                    break;
                }

                if (dist(gen) < (crossProbability / (1.0 + static_cast<float>(parent))))
                {
                    parents.push_back(population[parent]);
                }
            }
        }

        // crossing - crossing parents
        std::vector<Individual> newPopulation{};
        for (size_t i = 0; i < population.size() / 2; i++)
        {
            newPopulation.push_back(population[i]);
        }

        std::uniform_int_distribution<size_t> parentDist(0, parents.size() - 1);
        for (size_t i = 0; i < population.size() / 4; i++)
        {
            size_t parent1Index = parentDist(gen);
            size_t parent2Index = parentDist(gen);
            while (parent1Index == parent2Index)
            {
                parent2Index = parentDist(gen);
            }

            auto children = ox(parents[parent1Index], parents[parent2Index], gen);
            children.first.second = cycleWeight(points, children.first.first);
            children.second.second = cycleWeight(points, children.second.first);
            newPopulation.push_back(children.first);
            newPopulation.push_back(children.second);
        }

        // mutation
        for (auto &parent : newPopulation)
        {
            std::uniform_real_distribution<float> mutDist(0.0f, 1.0f);
            std::uniform_int_distribution<size_t> indexDist(1, parent.first.size() - 2);

            // reverse sequence mutation
            if (mutDist(gen) < mutationProbability)
            {
                size_t start = indexDist(gen);
                size_t end = indexDist(gen);

                if (start > end)
                {
                    std::swap(start, end);
                }

                std::reverse(parent.first.begin() + start, parent.first.begin() + end + 1);
                parent.second = cycleWeight(points, parent.first);
            }
            /*
                        // single swap mutation
                        if (mutDist(gen) < mutationProbability)
                        {
                            size_t start = indexDist(gen);
                            size_t end = indexDist(gen);

                            if (start > end)
                            {
                                std::swap(start, end);
                            }

                            std::swap(parent.first[start], parent.first[end]);
                            parent.second = cycleWeight(points, parent.first);
                        }
                        */
        }

        for (auto &parent : newPopulation)
        {
            localSearch(parent.first, points);
            parent.second = cycleWeight(points, parent.first);
        }

        totalCost += newPopulation.front().second;
        population = newPopulation;
    }

    MakeResults results;
    results.minWeight = population.front().second;
    results.avgWeight = (totalCost / iterationsNumber);
    results.timePerRepetition = 0.0;

    return results;
}

int main()
{
    std::cout << "Genetic Algorithm (OX)" << std::endl;
    for (const auto &readFile : readFiles)
    {
        std::vector<Points> points;
        const std::string readFileName = readPointsFromFile(readFile, points);

        std::cout << "File: " << readFileName << std::endl;

        constexpr size_t iter{100};
        const Graph_I graph = initializeGraph(points);

        std::mt19937 randomEngine{std::random_device{}()};

        // Genetic Algorithm
        size_t populationSize = 100;
        std::vector<Individual> population;
        for (size_t i = 0; i < populationSize; ++i)
        {
            std::vector<size_t> cycle;
            for (size_t vertex = 1; vertex <= points.size(); ++vertex)
            {
                cycle.push_back(vertex);
            }
            std::shuffle(cycle.begin(), cycle.end(), randomEngine);
            int_fast32_t cycleWeightValue = cycleWeight(points, cycle);
            population.push_back({cycle, cycleWeightValue});
        }

        auto start = std::chrono::high_resolution_clock::now();

        MakeResults results = genetic(population, points, iter, 0.8, 0.1, randomEngine);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // Find the best solution in the final population
        Individual bestIndividual = *std::min_element(population.begin(), population.end(),
                                                      [](const Individual &a, const Individual &b)
                                                      { return a.second < b.second; });

        std::cout << "Average Cost: " << results.avgWeight << "\nMinimum Cost: " << results.minWeight << "\nBest Solution: ";

        for (size_t indexVertex : bestIndividual.first)
        {
            std::cout << indexVertex << " ";
        }

        std::cout << "\nTime elapsed: " << elapsed.count() << " seconds" << std::endl;
    }

    return 0;
}
