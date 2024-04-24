#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>

// Define a structure to represent coordinates
struct Coordinates {
  int x;
  int y;
};

// Function to calculate the distance between two coordinates
double calculateDistance(const Coordinates& a, const Coordinates& b) {
  return std::sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}

// Function to generate a random permutation of the given set of coordinates
std::vector<int> generateRandomPermutation(int n) {
  std::vector<int> permutation(n);
  for (int i = 0; i < n; ++i) {
    permutation[i] = i;
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(permutation.begin(), permutation.end(), gen);

  return permutation;
}

// Function to calculate the height of the MST for a given permutation
double calculateMSTHeight(const std::vector<Coordinates>& coordinates, const std::vector<int>& permutation) {
  int n = coordinates.size();

  // Create the distance matrix
  std::vector<std::vector<double>> distanceMatrix(n, std::vector<double>(n, 0));

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      distanceMatrix[i][j] = calculateDistance(coordinates[i], coordinates[j]);
    }
  }

  // Run Prim's algorithm to find the MST
  std::vector<int> parent(n);
  std::vector<double> key(n, std::numeric_limits<double>::max());
  std::vector<bool> mstSet(n, false);

  key[0] = 0;
  parent[0] = -1;

  for (int count = 0; count < n - 1; ++count) {
    int u = -1;
    for (int v = 0; v < n; ++v) {
      if (!mstSet[v] && key[v] < key[u]) {
        u = v;
      }
    }

    if (u == -1) {
      break;
    }

    mstSet[u] = true;

    for (int v = 0; v < n; ++v) {
      if (distanceMatrix[u][v] && !mstSet[v] && distanceMatrix[u][v] < key[v]) {
        key[v] = distanceMatrix[u][v];
        parent[v] = u;
      }
    }
  }

  // Calculate the height of the MST
  double maxHeight = 0;

  for (int i = 1; i < n; ++i) {
    int parentIndex = parent[i];
    double edgeWeight = distanceMatrix[parentIndex][i];

    maxHeight = std::max(maxHeight, edgeWeight);
  }

  return maxHeight;
}

// Function to perform the Local Search algorithm
void localSearch(const std::vector<Coordinates>& coordinates, int nPermutations) {
  double totalHeight = 0;
  double totalSteps = 0;
  double bestHeight = std::numeric_limits<double>::max();
  std::vector<int> bestPermutation;

  for (int permutationIndex = 0; permutationIndex < nPermutations; ++permutationIndex) {
    std::vector<int> permutation = generateRandomPermutation(coordinates.size());

    // Initialize the best solution
    double currentHeight = calculateMSTHeight(coordinates, permutation);

    // Perform local search
    bool improvement = true;
    int steps = 0;

    while (improvement) {
      improvement = false;

      for (int i = 1; i < coordinates.size(); ++i) {
        for (int j = i + 1; j < coordinates.size(); ++j) {
          std::swap(permutation[i], permutation[j]);

          double newHeight = calculateMSTHeight(coordinates, permutation);
          if (newHeight < currentHeight) {
            currentHeight = newHeight;
            improvement = true;
          }

          std::swap(permutation[i], permutation[j]);
        }
      }

      steps++;
    }

    // Update total values
    totalHeight += currentHeight;
    totalSteps += steps;

    // Update best solution if the current one is better
    if (currentHeight < bestHeight) {
      bestHeight = currentHeight;
      bestPermutation = permutation;
    }
  }

  // Calculate averages
  double averageHeight = totalHeight / nPermutations;
  double averageSteps = totalSteps / nPermutations;

  // Output the results
  std::cout << "Average Height: " << averageHeight << std::endl;
  std::cout << "Average Steps: " << averageSteps << std::endl;
  std::cout << "Best Permutation: ";
  for (int i : bestPermutation) {
    std::cout << i << " ";
  }
  std::cout << "\nBest Height: " << bestHeight << std::endl;
}

int main() {
  // Coordinates of points
  std::vector<Coordinates> coordinates = {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}};

  // Number of permutations to try
  int nPermutations = 5;

  // Perform local search
  localSearch(coordinates, nPermutations);

  return 0;
}
