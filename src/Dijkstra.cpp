#include "FlightGraph.h"
#include <math.h>
#include <queue>
// Has more arguments than necessary to conform to possible future implementaion of weight function
template <class Route>
double distance_weight(const typename FlightGraph<Route>::Airport & c1, const typename FlightGraph<Route>::Airport & c2, const int stops, const std::string & equip) {
    double lat1 = c1.latitude * M_PI/180; //Convert from degrees to radians for Trig functions
    double lat2 = c2.latitude * M_PI/180; 
    double deltaLat = (c2.latitude-c1.latitude) * M_PI/180; 
    double deltaLong = (c2.longitude-c1.longitude) * M_PI/180;
    
    double a = pow(sin(deltaLat/2), 2) + cos(lat1)*cos(lat2)*pow(sin(deltaLong/2), 2); //Calculating Haversine
    double b = 2 * atan2(sqrt(a), sqrt(1-a)); //Haversine Formula for Great Circle Distance
    return b * 6371; //Multiplied by earth radius to get distance in kilometers
}

int main(int argc, char * argv[]) {
  // default values for unspecified arguments
  std::string airportsFN = "data/airports.dat";
  std::string routesFN = "data/routes.dat";
  std::string originIATA = "\"ORD\""; // Chicago O'Hare International Airport
  std::string destIATA = "\"EMD\""; // Emerald Airport

  // Parses arguments to program
  // -a : airport file
  // -r : route file
  // -o : origin airport (IATA)
  // -d : destination airport (IATA)
  for (int i = 1; i < argc; i++) {
    if (std::string("-a").compare(argv[i]) == 0) {
      if (++i < argc) {
        airportsFN = argv[i];
      }
    }
    else if (std::string("-r").compare(argv[i]) == 0) {
      if (++i < argc) {
        routesFN = argv[i];
      }
    }
    else if (std::string("-o").compare(argv[i]) == 0) {
      if (++i < argc) {
        originIATA = argv[i];
        originIATA = '"' + originIATA + '"';
      }
    }
    else if (std::string("-d").compare(argv[i]) == 0) {
      if (++i < argc) {
        destIATA = argv[i];
        destIATA = '"' + destIATA + '"';
      }
    }
  }

  std::cout << "Using " << airportsFN << " for airports file and " << routesFN << " for routes file." << std::endl;

  // struct type to feed into FlightGraph. Templating FlightGraph allows it to be more efficient for other algorithms.
  struct Route
  {
    double weight;
    int dest;
    // Route initializer to be used from FlightGraph.
    // passes most parameters to weight function to generate edge weight.
    Route(const FlightGraph<Route>::Airport & origin, const FlightGraph<Route>::Airport & dest, int destID, int stops, const std::string & equip) : dest(destID) {
      weight = distance_weight<Route>(origin, dest, stops, equip);
    }
  };

  // construct the FlightGraph struct with the openflight airport dataset
  FlightGraph<Route> fg(airportsFN);
  // provide routes with openflights routes dataset
  fg.addRoutes(routesFN);

  // make sure inputted airport IDs are valid
  if (fg.IATAMap.count(originIATA) == 0) {
    std::cout << "Origin IATA code " << originIATA << " is invalid." << std::endl;
    return 1;
  }
  if (fg.IATAMap.count(destIATA) == 0) {
    std::cout << "Destination IATA code " << destIATA << " is invalid." << std::endl;
    return 1;
  }
  
  const int originID = fg.IATAMap[originIATA];
  const int destID = fg.IATAMap[destIATA];
  
  std::cout << "Finding shortest path from " << fg.airports[originID].name << " to " << fg.airports[destID].name << "." << std::endl;
  
  // struct to hold all data required by Dijkstra algorithm
  struct NodeData {
    bool visited;
    double dist;
    int prev;
    NodeData() : visited(false), dist(std::numeric_limits<double>::infinity()), prev(-1) {}
    bool operator<(const NodeData & rhs) const {
      return this->dist < rhs.dist;
    }
  };
  
  // vector holds a NodeData struct for each airport.
  std::vector<NodeData> nodes(fg.airports.size(), NodeData());

  // a comparison function to pass to priority_queue. 
  // captures nodes vector because all actual dist values are stored there.
  auto cmp = [&nodes](int lhs, int rhs){return nodes[rhs] < nodes[lhs];};
  // priority_queue that pops out the node with smallest distance
  std::priority_queue<int, std::vector<int>, decltype(cmp)> pq(cmp);

  // Dijkstra's algo
  nodes[originID].dist = 0.0;
  pq.push(originID);

  while (!pq.empty()) {
    const int ID = pq.top();
    pq.pop();
    NodeData & node = nodes[ID];

    if (ID == destID) break;
    if (node.visited) continue;

    for (const Route & route : fg.airports[ID].routes) {
      NodeData & dest = nodes[route.dest];
      if (dest.visited) continue;
      const double altDist = node.dist + route.weight;
      if (altDist < dest.dist) {
        dest.dist = altDist;
        dest.prev = ID;
        pq.push(route.dest);
      }
    }
    node.visited = true;
  }

  // Finished traversal, check if successfull
  if (nodes[destID].prev == -1) {
    std::cout << "No route found." << std::endl;
    return 1;
  }
  // backtrace to collect path to dest
  std::vector<int> path;
  for (int ID = destID; ID != -1; ID = nodes[ID].prev) {
    path.push_back(ID);
  }
  for (auto ID = path.rbegin(); ID != path.rend(); ++ID) {
    std::cout << fg.airports[*ID].name << " -> ";
  }
  std::cout << "\b\b\b   " << std::endl;

  return 0;
}
