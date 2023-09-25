/*
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix);
double findStreetLength(StreetIdx street_id);
POIIdx findClosestPOI(LatLon my_position, std::string POItype);
std::string getOSMNodeTagValue (OSMID OSMid, std::string key);
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <map>
#include <float.h>
#include "m1.h"
#include "globals.h"
#include "OSMDatabaseAPI.h"

// Returns all street ids corresponding to street names that start with the 
// given prefix 
// The function should be case-insensitive to the street prefix. 
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to 
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty 
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty 
// (length 0) string, but your program must not crash if street_prefix is a 
// length 0 string.
// Speed Requirement --> high 

// std::unordered_map<std::string, std::vector> street_map;
// key: std::string street_prefix
// value: std::vector StreetIdx
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix) {

    std::string new_prefix = "";

    // remove spaces and lowercase all Chars
    for (int i = 0; i < street_prefix.length(); ++i) {
        if (street_prefix.at(i) == ' ') {
            continue;
        }
        new_prefix.push_back(tolower(street_prefix[i]));
    }

    if (new_prefix == "") {
        std::vector<StreetIdx> Empty;
        return Empty;
    }

    // std::unordered_map<std::string, std::set<StreetIdx>> street_map;
    std::vector<StreetIdx> street_map_vector(
        street_map[new_prefix].begin(), 
        street_map[new_prefix].end()
    );

    return street_map_vector;
}


// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength(StreetIdx street_id) {
    // Idea: preload a map of street_id : vector<street_segment_id>, std::find the correct street, loop over and sum the lengths of the segments
    double street_length = 0;

    std::unordered_map<StreetIdx, std::vector <StreetSegmentIdx>>::iterator found_street = segments_of_streets.find(street_id);
    std::vector<StreetSegmentIdx> segment_vector = found_street -> second;

    for (auto itr = segment_vector.begin(); itr != segment_vector.end(); itr++) {
        street_length = street_length + findStreetSegmentLength(*itr);
    }
    
    return street_length;
}


// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string POItype) {
    // Find all nodes with POItype, calculate all of their distances to my_position, return shortest one
    
    POIIdx min_dist_index = 0;
    double min_dist = DBL_MAX;

    for (int poi_index = 0; poi_index < getNumPointsOfInterest(); poi_index++) {
        if (getPOIType(poi_index) == POItype) {
            double cur_dist = findDistanceBetweenTwoPoints(getPOIPosition(poi_index), my_position);
            if (cur_dist < min_dist) {
                min_dist = cur_dist;
                min_dist_index = poi_index;
            }
        }
    }

    return min_dist_index;
}

// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is 
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue (OSMID OSMid, std::string key) {
    
    std::unordered_map<OSMID, const OSMNode*>::iterator osm_itr= osm_map.find(OSMid);
    std::string found_value;

    const OSMNode* found_node = osm_itr -> second;
    for (int i = 0; i < getTagCount(found_node); i++) {
        std::string node_key, node_value;
        std::tie(node_key, node_value) = getTagPair(found_node, i);
        
        if (node_key == key) {
            found_value = node_value;
        }
    }

    return found_value; 
}



