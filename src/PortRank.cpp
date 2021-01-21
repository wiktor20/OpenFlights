#include "FlightGraph.h"
//#include <execution>
#include <cmath>
#include <algorithm>
#include <numeric>

int main(int argc, char * argv[]) {
  std::string airports = "data/airports.dat"; // TODO: need to change to normal once done testing
  std::string routes = "data/routes.dat";
  std::string iterations_str = "20";
  std::string damping_str = "0.85";

  // Parses arguments to program
  // -a : airport file
  // -r : route file
  // -i : iterations
  // -d : damping factor
  for (int i = 1; i < argc; i++) {
    if (std::string("-a").compare(argv[i]) == 0) {
      if (++i < argc) {
        airports = argv[i];
      }
    }
    else if (std::string("-r").compare(argv[i]) == 0) {
      if (++i < argc) {
        routes = argv[i];
      }
    }
    else if (std::string("-i").compare(argv[i]) == 0) {
      if (++i < argc) {
        iterations_str = argv[i];
      }
    }
    else if (std::string("-d").compare(argv[i]) == 0) {
      if (++i < argc) {
        damping_str = argv[i];
      }
    }
  }

  int iterations = std::stoi(iterations_str);
  double d = std::stod(damping_str);
  if (d > 1.0) {
    std::cout << "Damping factor should by less than 1" << std::endl;
    return 1;
  }
  std::cout << "Running " << iterations_str << " iterations. Using damping factor " << d << "." << std::endl;

  std::cout << "Using " << airports << " for airports file and " << routes << " for routes file." << std::endl;
  
  // Route struct. only holds dest, since weights aren't needed for PageRank
  class Route {
    public:
    int dest;
    
    Route(const FlightGraph<Route>::Airport & origin, const FlightGraph<Route>::Airport & dest, int destID, int stops, const std::string & equip) : dest(destID) {}
  };

  // Initialize FlightGraph object
  FlightGraph<Route> fg(airports);
  fg.addRoutes(routes);

  // Start PageRank algo
  // store number of airports, since we'll be using it a lot
  size_t N = fg.airports.size();
  
  // create adjacency matrix. 
  std::vector<double> M(N * N, (1.0 - d) / (double)N);
  
  // fill adjacentcy matrix with data from FlightGraph
  for (size_t j = 0; j < N; j++) {
    const auto & routes = fg.airports[j].routes;
    // find all unique destinations (PageRank typically ignores multiple links to same site)
    std::vector<int> uniqueDests;
    uniqueDests.reserve(routes.size());
    std::transform(routes.cbegin(), routes.cend(), std::back_inserter(uniqueDests), 
                   [](Route r) -> int { return r.dest; });
    std::sort(uniqueDests.begin(), uniqueDests.end());
    uniqueDests.erase(std::unique(uniqueDests.begin(), uniqueDests.end()), uniqueDests.end());

    // mark edge in matrix. divide by num routes and modify so that it's principal eigenvector is R
    for (int dest : uniqueDests) {
      M[j + dest * N] += (d / (double)(uniqueDests.size()));
    }
  }
  
  // Create the PageRank vector R and initialize with random double from [0, 1]
  std::vector<double> R(N);
  std::generate(R.begin(), R.end(), []() -> double {return (double)(std::rand() % 101) / 100.0;});
  // divide each value of R by its first order normal
  double norm = std::accumulate(R.cbegin(), R.cend(), 0.0);
  std::for_each(R.begin(), R.end(), [norm](double & val) { val /= norm; });
  
  // multiply M into R [iteration] times
  for (int iter = 0; iter < iterations; iter++) {
    std::vector<double> nR(R.size(), 0.0);
    // matrix vector multiplication
    for (size_t i = 0; i < N; i++) {
      nR[i] = std::inner_product(R.begin(), R.end(), M.begin() + i * N, 0.0);
    }
    R = nR;
  }

  // make vector that can store airport IDs and associated PageRank values for sorting
  std::vector<std::pair<double, int>> ranks;
  ranks.reserve(N);
  for (size_t i = 0; i < N; i++) {
    ranks.push_back(std::pair(R[i], (int)i));
  }
  std::sort(ranks.begin(), ranks.end(), std::greater());
  
  // print out ordered airports
  std::cout << std::endl;
  for (auto AP : ranks) {
    std::cout << fg.airports[AP.second].name << ": " << AP.first << std::endl; // Output list of airports
  }




  return 0;
}
