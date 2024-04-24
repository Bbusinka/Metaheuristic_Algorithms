#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>

using namespace std;

struct Point
{
    int64_t x, y;
};

struct Edge
{
    int64_t u, v;
    double weight;
};


int64_t calculateDistance(const Point &a, const Point &b)
{
    return static_cast<int64_t>(round(sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y))));
}

vector<Point> readPointsFromFile(const string &filename)
{
    ifstream file(filename);
    vector<Point> points;
    int64_t vertex, x, y;

    while (file >> vertex >> x >> y)
    {
        points.push_back({x, y});
    }

    file.close();
    return points;
}

vector<Edge> MST(const vector<Point> &points)
{
    vector<Edge> edges;

    for (size_t i = 0; i < points.size(); ++i)
    {
        for (size_t j = i + 1; j < points.size(); ++j)
        {
            edges.push_back({static_cast<int64_t>(i), static_cast<int64_t>(j), static_cast<double>(calculateDistance(points[i], points[j]))});
        }
    }

    sort(edges.begin(), edges.end(), [](const Edge &a, const Edge &b)
         { return a.weight < b.weight; });

    vector<int64_t> parent(points.size(), -1);
    vector<Edge> mstEdges;

    for (const Edge &edge : edges)
    {
        int64_t u = edge.u;
        int64_t v = edge.v;
        int64_t uParent = u;
        int64_t vParent = v;

        while (parent[uParent] != -1)
        {
            uParent = parent[uParent];
        }

        while (parent[vParent] != -1)
        {
            vParent = parent[vParent];
        }

        if (uParent != vParent)
        {
            mstEdges.push_back(edge);
            parent[uParent] = vParent;
        }
    }

    return mstEdges;
}


pair<vector<Point>, uint64_t> LocalSearch(const vector<Point> &start_point, mt19937 &randomEngine)
{
    vector<Point> current_solution = start_point;
    uint64_t steps = 0;
    vector<Point> best_solution;

    while (true)
    {
        ++steps;
        int64_t min_diff = numeric_limits<int64_t>::max();
        size_t min_i = 0;
        size_t min_k = 0;

        for (size_t i = 0; i < current_solution.size(); ++i)
        {
            for (size_t k = i + 1; k < current_solution.size(); ++k)
            {
                if ((k - i) + 2 >= current_solution.size())
                    continue;

                int64_t diff = 0;

                if (i == 0)
                {
                    diff -= calculateDistance(current_solution[i], current_solution[current_solution.size() - 1]);
                    diff += calculateDistance(current_solution[k], current_solution[current_solution.size() - 1]);
                }
                else
                {
                    diff -= calculateDistance(current_solution[i - 1], current_solution[i]);
                    diff += calculateDistance(current_solution[i - 1], current_solution[k]);
                }

                if (k + 1 == current_solution.size())
                {
                    diff -= calculateDistance(current_solution[0], current_solution[k]);
                    diff += calculateDistance(current_solution[0], current_solution[i]);
                }
                else
                {
                    diff -= calculateDistance(current_solution[k + 1], current_solution[k]);
                    diff += calculateDistance(current_solution[k + 1], current_solution[i]);
                }

                if (min_diff > diff)
                {
                    min_diff = diff;
                    min_i = i;
                    min_k = k;
                }
                else if (min_diff == diff && uniform_int_distribution<int>(0, 1)(randomEngine) == 0)
                {
                    min_i = i;
                    min_k = k;
                }
            }
        }

        if (min_diff == 0)
        {
            best_solution = current_solution;
            break;
        }

        while (min_i < min_k)
        {
            swap(current_solution[min_i], current_solution[min_k]);
            min_i += 1;
            min_k -= 1;
        }
    }

    return {best_solution, steps};
}

int main()
{
    vector<Point> points = readPointsFromFile("1.txt");

    vector<Edge> mstEdges = MST(points);

    int64_t mstWeight = 0;
    for (const Edge &edge : mstEdges)
    {
        mstWeight += edge.weight;
    }

    cout << "MST: " << mstWeight << endl;

    size_t n = ceil(sqrt(points.size()));
    mt19937 randomEngine(random_device{}());

    vector<uint64_t> steps_values;
    vector<uint64_t> costs;

    vector<Point> best_solution;
    uint64_t minimum_cost = numeric_limits<uint64_t>::max();

    for (size_t i = 0; i < n; ++i)
    {
        vector<Point> shuffled_points = points;
        shuffle(shuffled_points.begin() + 1, shuffled_points.end(), randomEngine);

        auto result = LocalSearch(shuffled_points, randomEngine);
        steps_values.push_back(result.second);

        uint64_t cost = 0;
        for (size_t j = 1; j < result.first.size(); ++j)
        {
            cost += calculateDistance(result.first[j - 1], result.first[j]);
        }

        costs.push_back(cost);

        if (cost < minimum_cost)
        {
            minimum_cost = cost;
            best_solution = result.first;
        }
    }

    double average_steps = accumulate(steps_values.begin(), steps_values.end(), 0.0) / n;
    double average_cost = accumulate(costs.begin(), costs.end(), 0.0) / n;

    cout << "Average Steps: " << average_steps << endl;
    cout << "Average Cost: " << average_cost << endl;
    cout << "Minimum Cost: " << minimum_cost << endl;

    return 0;
}
