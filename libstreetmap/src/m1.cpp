/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <cctype>
#include <map>
#include <string>
#include <thread>

#include "m1.h"
#include "globals.h"
#include "load_helpers.h"
#include "OSMDatabaseAPI.h"
#include "appGlobals.h"

// Global variables
// See detailed definitions and functionalities in globals.h

// -------------------------Milestone 1-------------------------------------------------------
std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;

std::unordered_map<std::string, std::set<StreetIdx>> street_map;

std::unordered_map<OSMID, const OSMNode*> osm_map;

std::unordered_map<StreetIdx, std::vector<StreetSegmentIdx>> segments_of_streets;

std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> intersections_of_streets;

std::vector<double> street_segment_length;

std::vector<float> street_segment_time;


// -------------------------Milestone 2-------------------------------------------------------
std::vector<intersection_data> intersections;

std::vector<double> map_bounds(4);

std::vector<double> xy_bounds(4);

double avg_lat_global;

std::vector<street_segment_data> segments;

std::vector<street_segment_data> segments_zoom_small;

std::vector<street_segment_data> segments_zoom_0;

std::vector<street_segment_data> segments_zoom_1;

std::vector<street_segment_data> segments_zoom_2;

std::vector<street_segment_data> segments_zoom_3;

std::vector<street_segment_data> segments_zoom_4;

std::vector<poi_data> poi_zoom_small;

std::vector<poi_data> poi_zoom_0;

std::vector<poi_data> poi_zoom_1;

std::unordered_map<OSMID, const OSMWay*> osm_way_map;

std::unordered_map<OSMID, std::string> osmnode_values;

std::vector<const OSMNode*> subway_osm_nodes;

std::unordered_map<const OSMNode*, subway_info> subway_node_info;

int num_zoom_small;

int num_zoom_0;

int num_zoom_1;

int num_zoom_2;

int num_zoom_3;

int num_zoom_4;

std::set<StreetIdx> street_id_small;

std::set<StreetIdx> street_id_0;

std::set<StreetIdx> street_id_1;

std::set<StreetIdx> street_id_2;

std::set<StreetIdx> street_id_3;

std::set<StreetIdx> street_id_4;

std::unordered_map<StreetIdx, std::pair<ezgl::point2d, ezgl::point2d>> street_points;

std::vector<feature_data> features;

std::vector<feature_data> features_park;

std::vector<feature_data> features_beach;

std::vector<feature_data> features_lake;

std::vector<feature_data> features_river;

std::vector<feature_data> features_island;

std::vector<feature_data> features_building;

std::vector<feature_data> features_greenspace;

std::vector<feature_data> features_golfcourse;

std::vector<feature_data> features_stream;

std::vector<feature_data> features_glacier;

std::vector<POI_data> POIs;

bool reload_map = false;

highlight_data highlight_global;

show_elements show_elements_global;

// -------------------------Milestone 3-------------------------------------------------------
std::vector<Node> nodes;

// load everything in the nodes

bool search_activated;

finding_path found_path_global;

std::vector<std::string> instructions;

// -------------------------Milestone 4-------------------------------------------------------
double trip_travel_time;

// std::vector<const OSMNODE*> osm_subway_stations;

// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.
bool loadMap(std::string map_streets_database_filename) {

    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);

    if (!load_successful) {
        return load_successful;
    }

    // replace .streets to .osm
    // https://www.tutorialspoint.com/replace-part-of-a-string-with-another-string-in-cplusplus#:~:text=In%20C%2B%2B%20the%20replacing%20is,first%20occurrence%20of%20the%20match.
    std::string OSM_filename = map_streets_database_filename;
    std::string replace_str = ".osm";

    int index = OSM_filename.find(".streets");
    if (index != std::string::npos) {
        OSM_filename.replace(index, 8 , replace_str);
    }
    bool OSM_load = loadOSMDatabaseBIN(OSM_filename);
    if (!OSM_load) {
        return OSM_load;
    }

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    std::cout << "loadOSM: " << OSM_filename << std::endl;

    //
    // Load your map related data structures here.
    //

    // Create a map of OSM ids and their corresponding key,value pairs (node)
    load_osm_map();
    
    // Create a vector matrix of all street intersections and their attached street segments
    load_intersection_street_segments();
    // Create a hash map of all street names and their corresponding IDs
    // The street names will have spaces removed and be lowercased
    // 
    // std::unordered_map<std::string, std::set<StreetIdx>> street_map;
    // key: std::string street_prefix
    // value: std::set <StreetIdx>
    
    load_street_map();
    // create an unordered map for all street segments
    // load the street names into an unordered_map
    // std::unordered_map<std::string, std::vector <StreetSegmentIdx>> segments_of_streets;
    // key: streetID
    // value: a vector of StreetSegmentIdx
    load_segments_of_streets();
    // create an unordered map for all intersections on a street
    // std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> intersections_of_streets;
    load_intersections_of_streets();

    // create two vectors of int
    // std::vector<double> street_segment_length;
    // std::vector<float> street_segment_time;
    // stores the length and travel_time of each segment
    load_street_segment_length();

    // m2 preload
    load_osm_ways();
    
    load_intersections();

    load_osm_way_map();

    load_segments();

    load_pois();

    load_features();

    load_POIs();

    load_subway_stations();

    load_nodes();
    
    highlight_global.from_highlighted = false;
    highlight_global.from_id = -1;
    highlight_global.to_highlighted = false;
    highlight_global.to_id = -1;
    highlight_global.two_streets_intersections.clear();
    highlight_global.all_intersections = false;
    highlight_global.single_street_searched = -1;
    highlight_global.from_highlighted = false;
    highlight_global.from_id = -1;
    highlight_global.to_highlighted = false;
    highlight_global.to_id = -1;
    highlight_global.two_streets_intersections.clear();
    highlight_global.all_intersections = false;
    highlight_global.single_street_searched = -1;


    show_elements_global.dark = false;
    show_elements_global.show_poi = false;
    show_elements_global.subways = false;

    search_activated = false;

    found_path_global.draw_found_path = false;
    found_path_global.found_path.clear();

    instructions.clear();


    return load_successful;
}

void closeMap() {

    //Clean-up your map related data structures here

    intersection_street_segments.clear();

    street_map.clear();

    osm_map.clear();

    segments_of_streets.clear();

    intersections_of_streets.clear();

    street_segment_length.clear();

    street_segment_time.clear();

    subway_osm_nodes.clear();

    subway_node_info.clear();

    closeStreetDatabase();
    
    closeOSMDatabase();

    return;
}

