#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

template <class Route>
class FlightGraph {
  public:
  struct Airport {
    std::string name, city, country;
    double latitude, longitude;
    std::vector<Route> routes;
  };
  // the only purpose of this class is to populate this vector
  // internal internal IDs for airports are indices for this vector.
  std::vector<Airport> airports;
  
  // allows for nicer terminal input. 
  std::unordered_map<std::string, int> IATAMap;
  // map from OpenFlight ID to internal ID. Internal IDs are consecutive, while OFIDs have gaps
  // since most OFIDs are consecutive, we could probably derive internal IDs in more efficient way, 
  // but i don't want to do it.
  std::unordered_map<int, int> OFIDMap;

  // initializes an airport structs for each airport in OpenFlights dataset
  FlightGraph(const std::string & airportsFilename) {
    std::ifstream airportsFile(airportsFilename);
    if (!airportsFile.is_open()) {
      std::cerr << "error: file open failed " << airportsFilename << std::endl;
      exit(1);
    }
    std::string line;
    for (int i = 0; std::getline(airportsFile, line); i++)
    {
      // This is very gross, but the only simple way to parse the file,
      std::stringstream ss(line);
      std::string OFID_str, name, city, country, IATA, ICAO, 
                  latitude_str, longitude_str, altitude_str, 
                  timezone_str, DST, tz, type, source;
      
      // reading until I see a comma and storing in a string. 
      // For some values, I read until I see a '"' instead, since there might be commas inside.
      std::getline(ss, OFID_str, ',');
      ss.ignore(2, '"');
      std::getline(ss, name, '"');
      ss.ignore(256, ',');
      ss.ignore(2, '"');
      std::getline(ss, city, '"');
      ss.ignore(256, ',');
      ss.ignore(2, '"');
      std::getline(ss, country, '"');
      ss.ignore(256, ',');
      std::getline(ss, IATA, ',');
      std::getline(ss, ICAO, ',');
      std::getline(ss, latitude_str, ',');
      std::getline(ss, longitude_str, ',');
      std::getline(ss, altitude_str, ',');
      std::getline(ss, timezone_str, ',');
      ss.ignore(2, '"');
      std::getline(ss, DST, '"');
      ss.ignore(256, ',');
      ss.ignore(2, '"');
      std::getline(ss, tz, '"');
      ss.ignore(256, ',');
      ss.ignore(2, '"');
      std::getline(ss, type, '"');
      ss.ignore(256, ',');
      ss.ignore(2, '"');
      std::getline(ss, source, '"');
      ss.ignore(256, ',');
      

      int OFID = stoi(OFID_str);
      // Don't need right now, but might be helpful later
      //int altitude = stoi(altitude_str);
      double latitude = stod(latitude_str);
      double longitude = stod(longitude_str);
      
      // initialize Airport struct at end of vector
      airports.emplace_back(Airport {name, city, country, latitude, longitude, std::vector<Route>()});
      IATAMap[IATA] = i;
      OFIDMap[OFID] = i;
    }
    airportsFile.close();
  }
  
  // reads from openflights routes dataset and populates routes vector in each Airport struct
  bool addRoutes(const std::string & routesFilename) {
    std::ifstream routesFile(routesFilename);
    if (!routesFile.is_open()) {
      std::cerr << "error: file open failed " << routesFilename << std::endl;
      return false;
    }
    std::string line;
    while (std::getline(routesFile, line)) {
      std::stringstream ss(line);
      std::string airline, airlineID_str, srcIATA, srcOFID_str, 
                  destIATA, destOFID_str, codeshare, stops_str, equip;

      // much easier parsing here, since there aren't complex names with internal commas or ""
      std::getline(ss, airline, ',');
      std::getline(ss, airlineID_str, ',');
      std::getline(ss, srcIATA, ',');
      std::getline(ss, srcOFID_str, ',');
      std::getline(ss, destIATA, ',');
      std::getline(ss, destOFID_str, ',');
      std::getline(ss, codeshare, ',');
      std::getline(ss, stops_str, ',');
      std::getline(ss, equip, ',');       // equip denotes planes used. could be useful in determining throughput
      // ignore any route from or to an airport without a defined openflights ID
      if (srcOFID_str != "\\N" && destOFID_str != "\\N") {
        int srcOFID = stoi(srcOFID_str);
        int destOFID = stoi(destOFID_str);
        // not necessary atm, but might be useful in the future
        //int airlineID = stoi(airlineID_str);
        //int stops = stoi(stops_str);

        // do not reference any airports that aren't already on the map.
        if (OFIDMap.count(srcOFID) != 0 && OFIDMap.count(destOFID) != 0) {
          int srcID = OFIDMap[srcOFID];
          int destID = OFIDMap[destOFID];
          airports[srcID].routes.emplace_back(Route(airports[srcID], airports[destID], destID, 1, "747"));
        }
      }
    }
    return true;
  }
};
