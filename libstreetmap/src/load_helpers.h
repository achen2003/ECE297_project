// put all the useful helper functions here

#include <iostream>
#include <cctype>
#include <vector>
#include <unordered_set>
#include <set>
#include "climits"


#include "StreetsDatabaseAPI.h"
#include "m1_helpers.h"
#include "m1.h"
#include "OSMDatabaseAPI.h"
#include "globals.h"


// Load helpers
// The details of the loaded data structure could be found in the globals.h file

// -------------------------Milestone 1-------------------------------------------------------

void load_osm_map();

void load_intersection_street_segments();


// The street names will have spaces removed and be lowercased
//
// std::unordered_map<std::string, std::set<StreetIdx>> street_map;
// key: std::string street_prefix
// value: std::set <StreetIdx>
void load_street_map();


// create an unordered map for all street segments
// load the street names into an unordered_map
// std::unordered_map<std::string, std::vector <StreetSegmentIdx>> segments_of_streets;
// key: streetID
// value: a vector of StreetSegmentIdx
void load_segments_of_streets();


// create an unordered map for all intersections on a street
// key: streetID
// value: the vector of all intersections of the street
// std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> intersections_of_streets;
void load_intersections_of_streets();


// create two vectors of int
// std::vector<double> street_segment_length;
// std::vector<float> street_segment_time;
// stores the length and travel_time of each segment
void load_street_segment_length();

// -------------------------Milestone 2-------------------------------------------------------

// load vector map_bounds
// 0: max_lat
// 1: min_lat
// 2: max_lon
// 3: min_lon
// avg_lat
// 
// create a vector of intersection structures
// load the min/max latlon
// std::vector<intersection_data> intersections;
// stores LatLon position, xy_loc and intersection name in each structure
//
// typedef struct {
//    LatLon position;
//    ezgl::point2d xy_loc;
//    std::string name;
// } intersection_data;
void load_intersections();

void load_segments();

void load_pois();

void load_osm_way_map();

void load_osm_ways();

void load_subway_entrances();

void load_features();

void load_POIs();

void load_subway_stations();

void load_nodes();