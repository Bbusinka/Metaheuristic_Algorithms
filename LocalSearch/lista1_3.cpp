#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>

using namespace std;

struct Vertex {
    int id;
    double x;
    double y;
};

// Funkcja obliczająca odległość euklidesową między dwoma wierzchołkami
double calculateDistance(const Vertex& v1, const Vertex& v2) {
    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;
    return sqrt(dx * dx + dy * dy);
}

// Funkcja obliczająca koszt trasy komiwojażera dla danej permutacji wierzchołków
double calculateTotalDistance(const vector<Vertex>& vertices, const vector<int>& permutation) {
    double totalDistance = 0.0;
    for (int i = 0; i < permutation.size() - 1; ++i) {
        totalDistance += calculateDistance(vertices[permutation[i]], vertices[permutation[i + 1]]);
    }
    return totalDistance;
}

// Funkcja do zamiany miejscami dwóch wierzchołków w permutacji
void swapVertices(vector<int>& permutation, int i, int j) {
    swap(permutation[i], permutation[j]);
}

// Algorytm Local Search z wyborem tylko najlepszego sąsiada spośród n losowo wybranych
void localSearchModified(const vector<Vertex>& vertices, int iterations, int neighborsToConsider) {
    srand(time(0));

    int n = vertices.size();

    // Inicjalizacja permutacji początkowej
    vector<int> currentSolution(n);
    for (int i = 0; i < n; ++i) {
        currentSolution[i] = i;
    }

    // Obliczenie kosztu dla permutacji początkowej
    double currentCost = calculateTotalDistance(vertices, currentSolution);
    double bestCost = currentCost;
    vector<int> bestSolution = currentSolution;

    double totalBestCost = 0.0;
    int totalImprovementSteps = 0;
    int iterationsWithImprovement = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        // Wybierz losowe wierzchołki do zamiany miejscami
        int i = rand() % n;
        int j = rand() % n;

        // Zastosuj ruch i oblicz nowy koszt
        vector<int> candidateSolution = currentSolution;
        swapVertices(candidateSolution, i, j);
        double candidateCost = calculateTotalDistance(vertices, candidateSolution);

        // Wybierz kandydata do poprawy spośród n losowo wybranych sąsiadów
        for (int k = 0; k < neighborsToConsider - 1; ++k) {
            i = rand() % n;
            j = rand() % n;
            vector<int> neighborSolution = currentSolution;
            swapVertices(neighborSolution, i, j);
            double neighborCost = calculateTotalDistance(vertices, neighborSolution);
            if (neighborCost < candidateCost) {
                candidateSolution = neighborSolution;
                candidateCost = neighborCost;
            }
        }

        // Wybierz kandydata do poprawy spośród n losowo wybranych sąsiadów
        if (candidateCost < currentCost) {
            currentSolution = candidateSolution;
            currentCost = candidateCost;
            totalImprovementSteps++;
        }

        // Zaktualizuj najlepsze rozwiązanie
        if (candidateCost < bestCost) {
            bestSolution = candidateSolution;
            bestCost = candidateCost;
        }

        // Zsumuj najlepszy koszt w każdej iteracji
        totalBestCost += bestCost;
    }

    // Oblicz średnią wartość uzyskanego rozwiązania
    double averageBestCost = totalBestCost / iterations;

    // Oblicz średnią liczbę kroków poprawy
    double averageImprovementSteps = (iterationsWithImprovement > 0) ?
        static_cast<double>(totalImprovementSteps) / iterationsWithImprovement : 0.0;

    // Wyświetl wyniki
    cout << "Najlepsze uzyskane rozwiazanie: ";
    for (int val : bestSolution) {
        cout << val << " ";
    }
    cout << "\nSrednia wartosc uzyskanego rozwiazania: " << averageBestCost << endl;
    cout << "Srednia liczba krokow poprawy: " << averageImprovementSteps << endl;
    cout << "Najlepsza uzyskana wartosc funkcji celu: " << bestCost << endl;
}

int main() {
    // Wczytaj dane z pliku wejściowego
    ifstream inputFile("1.txt");
    if (!inputFile.is_open()) {
        cerr << "Nie udalo sie otworzyc pliku input.txt" << endl;
        return 1;
    }

    vector<Vertex> vertices;
    int id, x, y;
    while (inputFile >> id >> x >> y) {
        vertices.push_back({id, static_cast<double>(x), static_cast<double>(y)});
    }

    inputFile.close();

    int iterations = 10000;
    int neighborsToConsider = 5; // liczba sąsiadów do rozważenia

    // Uruchom zmodyfikowany algorytm Local Search
    localSearchModified(vertices, iterations, neighborsToConsider);

    return 0;
}
