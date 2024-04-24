#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <cmath>
#include <fstream>
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

int64_t metric(const Point &a, const Point &b)
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
            edges.push_back({static_cast<int64_t>(i), static_cast<int64_t>(j), static_cast<double>(metric(points[i], points[j]))});
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

pair<vector<Point>, uint64_t> LocalSearch(const vector<Point> &start_point)
{
    vector<Point> current_solution = start_point;
    uint64_t steps = 0;
    vector<Point> best_solution;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<size_t> dist(0, current_solution.size() - 1);
    size_t start_node = dist(gen);

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
                    diff -= metric(current_solution[i], current_solution[current_solution.size() - 1]);
                    diff += metric(current_solution[k], current_solution[current_solution.size() - 1]);
                }
                else
                {
                    diff -= metric(current_solution[i - 1], current_solution[i]);
                    diff += metric(current_solution[i - 1], current_solution[k]);
                }

                if (k + 1 == current_solution.size())
                {
                    diff -= metric(current_solution[0], current_solution[k]);
                    diff += metric(current_solution[0], current_solution[i]);
                }
                else
                {
                    diff -= metric(current_solution[k + 1], current_solution[k]);
                    diff += metric(current_solution[k + 1], current_solution[i]);
                }

                if (min_diff > diff)
                {
                    min_diff = diff;
                    min_i = i;
                    min_k = k;
                }
                else if (min_diff == diff && rand() % 2 == 0)
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

    uint64_t steps_sum = 0;
    uint64_t minimum_cost = numeric_limits<uint64_t>::max();
    uint64_t cost_sum = 0;
    vector<Point> best_solution;

    size_t times = ceil(sqrt(points.size()));

    for (size_t i = 0; i < times; ++i)
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<size_t> dist(0, points.size() - 1);
        size_t start_node = dist(gen);

        vector<Point> start_solution;
        for (size_t j = start_node; j < points.size(); ++j)
        {
            start_solution.push_back(points[j]);
        }
        for (size_t j = 0; j < start_node; ++j)
        {
            start_solution.push_back(points[j]);
        }

        auto result = LocalSearch(start_solution);
        steps_sum += result.second;

        uint64_t cost = 0;
        for (size_t j = 1; j < result.first.size(); ++j)
        {
            cost += metric(result.first[j - 1], result.first[j]);
        }

        cost_sum += cost;

        if (cost < minimum_cost)
        {
            minimum_cost = cost;
            best_solution = result.first;
        }
    }

    cout << "Average Steps: " << static_cast<double>(steps_sum) / times << endl;
    cout << "Average Cost: " << static_cast<double>(cost_sum) / times << endl;
    cout << "Minimum Cost: " << minimum_cost << endl;

    return 0;
}
