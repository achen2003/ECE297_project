#include <map>
#include <set>
#include <limits>
#include <queue>
#include "ezgl/point.hpp"
#include "m1.h"
#include "OSMDatabaseAPI.h"

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#pragma once

// Globals declared in m1
// ==============================================================================

// The global variable that stores all data loaded from the StreetsDatabaseAPI
// the real global variables are declared in m1.cpp

// A double vector of integers
// the stored integers are the street segment IDs
// Outer vector: collection of all intersections (points) of the city in the database
// Each intersection has a unique index 
// Inner vector: collection of all streets crossing that intersection
// aka: the street_segments from each intersection
extern std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;


// unordered map of the street prefixes
// key: the prefix string
// value: the set of all StreetIdx
// use set to prevent duplicates, will convert set to vector when needed
extern std::unordered_map<std::string, std::set<StreetIdx>> street_map;

// An unordered map of OSMID : OSMNode
// value: OSMNode*
// key: OSMID
extern std::unordered_map<OSMID, const OSMNode*> osm_map;

// value: vector of the street segments under the street name
// key: street index
extern std::unordered_map<StreetIdx, std::vector <StreetSegmentIdx>> segments_of_streets;


// value: vector of the intersections under the street name
// key: street index
extern std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> intersections_of_streets;

// store the street length and travel time of each segment in vectors
// Index: StreetSegmentIdx
extern std::vector<double> street_segment_length;
extern std::vector<float> street_segment_time;


// Globals declared in m2
// ==============================================================================

typedef struct {
   LatLon position;
   ezgl::point2d xy_loc;
   std::string name;
   bool highlight;
} intersection_data;

// Global variables
extern std::vector<intersection_data> intersections;

typedef struct {
   bool from_highlighted;
   IntersectionIdx from_id;
   bool to_highlighted;
   IntersectionIdx to_id;
   std::vector<int> mulId;
   bool all_intersections;
   std::vector<IntersectionIdx> two_streets_intersections;
   StreetIdx single_street_searched;
} highlight_data;
extern highlight_data highlight_global;






   // 0: max_lat
   // 1: min_lat
   // 2: max_lon
   // 3: min_lon
extern std::vector<double> map_bounds;
   //  xy_bounds[0] = x_from_lon(max_lon);
   //  xy_bounds[1] = x_from_lon(min_lon);
   //  xy_bounds[2] = y_from_lat(max_lat);
   //  xy_bounds[3] = y_from_lat(min_lat);
extern std::vector<double> xy_bounds;
extern double avg_lat_global;

typedef struct {
   int num_curve_p;
   StreetIdx street_id;
   bool one_way;
   bool major_road;
   float speed_limit;
   float length;
   double angle;
   std::string road_type;
   std::string street_name;
   ezgl::point2d from_xy;
   ezgl::point2d to_xy;
   ezgl::point2d middle_xy;
   std::vector<ezgl::point2d> seg_curve_points;
   std::vector<ezgl::point2d> arrow_vertices;
} street_segment_data;

extern std::vector<street_segment_data> segments;

extern std::unordered_map<StreetIdx, std::pair<ezgl::point2d, ezgl::point2d>> street_points;


// typedef struct {
//    std::string road_type;
//    ezgl::point2d xy_coord;
// } osm_node_data;

typedef struct {
   std::string type;
   std::string name;
   ezgl::point2d coord_xy;
} poi_data;

extern std::vector<poi_data> poi_zoom_small;

extern std::vector<poi_data> poi_zoom_0;

extern std::vector<poi_data> poi_zoom_1;

extern std::unordered_map<OSMID, const OSMWay*> osm_way_map;

extern std::unordered_map<OSMID, std::string> osmnode_values;

typedef struct {
   std::string subway_name;
   LatLon latlon_loc;
   ezgl::point2d xy_pos;
} subway_info;

extern std::vector<const OSMNode*> subway_osm_nodes;
extern std::unordered_map<const OSMNode*, subway_info> subway_node_info;

typedef struct {
   bool show_poi;
   bool dark;
   bool subways;
} show_elements;

extern show_elements show_elements_global;

extern std::vector<ezgl::point2d> street_middle_points;

extern std::vector<street_segment_data> segments_zoom_small;

extern std::vector<street_segment_data> segments_zoom_0;

extern std::vector<street_segment_data> segments_zoom_1;

extern std::vector<street_segment_data> segments_zoom_2;

extern std::vector<street_segment_data> segments_zoom_3;

extern std::vector<street_segment_data> segments_zoom_4;

extern std::set<StreetIdx> street_id_small;

extern std::set<StreetIdx> street_id_0;

extern std::set<StreetIdx> street_id_1;

extern std::set<StreetIdx> street_id_2;

extern std::set<StreetIdx> street_id_3;

extern std::set<StreetIdx> street_id_4;

extern int num_zoom_small;

extern int num_zoom_0;

extern int num_zoom_1;

extern int num_zoom_2;

extern int num_zoom_3;

extern int num_zoom_4; 


/*
const std::string&  getFeatureName(FeatureIdx featureIdx);
FeatureType         getFeatureType(FeatureIdx featureIdx);
TypedOSMID          getFeatureOSMID(FeatureIdx featureIdx);
int                 getNumFeaturePoints(FeatureIdx featureIdx);
LatLon              getFeaturePoint(FeatureIdx featureIdx, int pointNum);
*/
typedef struct {
   FeatureIdx feature_id;
   std::string feature_name;
   FeatureType feature_type;
   int feature_num_curve_points;
   std::vector<ezgl::point2d> feature_curve_points;
   double feature_area;
} feature_data;

extern std::vector<feature_data> features;

extern std::vector<feature_data> features_park;

extern std::vector<feature_data> features_beach;

extern std::vector<feature_data> features_lake;

extern std::vector<feature_data> features_river;

extern std::vector<feature_data> features_island;

extern std::vector<feature_data> features_building;

extern std::vector<feature_data> features_greenspace;

extern std::vector<feature_data> features_golfcourse;

extern std::vector<feature_data> features_stream;

extern std::vector<feature_data> features_glacier;


typedef struct {
   std::string POItype;
   std::string POIname;
   ezgl::point2d POI_point;
} POI_data;

extern std::vector<POI_data> POIs;

extern bool reload_map;



// Globals declared in m3
// ==============================================================================

// typedef pair<IntersectionIdx, double> intersection_pair;

// typedef pair<IntersectionIdx, double> intersection_pair;
typedef struct {
    IntersectionIdx nodeID;
    std::vector<StreetSegmentIdx> edges;
        // Outgoing edges etc.
    StreetSegmentIdx reachingEdge; // edge used to reach node double bestTime; // Shortest time found to this node so far
    bool visited;
    double bestTime;
} Node;

extern std::vector<Node> nodes;

struct WaveElem {
    IntersectionIdx nodeID;
    StreetSegmentIdx edgeID; // edge used to reach this node 
    double travelTime;       // Total travel time to reach node
    WaveElem (int n, int e, float time) {
    nodeID = n; edgeID = e; travelTime = time; }
};

// Used for the priority queue
struct CompareWaveElem {
    bool operator()(const WaveElem& we1, const WaveElem& we2) {
        return we1.travelTime > we2.travelTime;
    }
};

extern bool search_activated;

typedef struct {
   std::vector<StreetSegmentIdx> found_path;
   bool draw_found_path;
} finding_path;

extern finding_path found_path_global;

extern std::vector<std::string> instructions;

typedef struct {
    IntersectionIdx to_node;
    IntersectionIdx from_node;
    StreetSegmentIdx passing_edge;
} Path;

// Global variables used in M4

// Updated every time when find path is called
// Used as a temp storing variable
extern double trip_travel_time;



