#include "load_helpers.h"
#include "m2_helper.h"
#include "ezgl/point.hpp"
#include <cmath>
#include <utility>

// -------------------------Milestone 1-------------------------------------------------------


// Load helpers
void load_osm_map() {
    for (int osm_id = 0; osm_id < getNumberOfNodes(); osm_id++) {
        const OSMNode *osm_node = getNodeByIndex(osm_id);
        osm_map.insert({(osm_node -> id()), osm_node});
    }
}


void load_intersection_street_segments() {
    intersection_street_segments.resize(getNumIntersections());
    for (IntersectionIdx intersection = 0; intersection < getNumIntersections(); ++intersection) {
        for (int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
            int ss_id = getIntersectionStreetSegment(intersection, i);
            intersection_street_segments[intersection].push_back(ss_id);
        }
    }    
}


// The street names will have spaces removed and be lowercased
// std::unordered_map<std::string, std::set<StreetIdx>> street_map;
// key: std::string street_prefix
// value: std::set <StreetIdx>
void load_street_map() {
    for (int street_index = 0; street_index < getNumStreets(); street_index++) {
        std::string street_name = getStreetName(street_index);
        std::string new_name = "";
        // delete empty spaces and upper cases
        for (int i = 0; i < street_name.length(); i++) {
            if (street_name[i] == ' ') {
                continue;
            }
            new_name.push_back(tolower(street_name[i]));

        }
        // the street_name is now lower cases and without spaces

        for (int i = 1; i <= street_name.length(); i++) {
            std::string street_prefix = new_name.substr(0, i);
            street_map[street_prefix].insert(street_index);
        }
        
    }
}


// create an unordered map for all street segments
// load the street names into an unordered_map
// std::unordered_map<std::string, std::vector <StreetSegmentIdx>> segments_of_streets;
// key: streetID
// value: a vector of StreetSegmentIdx
void load_segments_of_streets() {
    for (StreetSegmentIdx segment_index = 0; segment_index < getNumStreetSegments(); ++segment_index ) {
        StreetSegmentInfo segment_info = getStreetSegmentInfo(segment_index); // structure
        segments_of_streets[segment_info.streetID].push_back(segment_index);
    }    
}


// create an unordered map for all intersections on a street
// key: streetID
// value: the vector of all intersections of the street
// std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> intersections_of_streets;
void load_intersections_of_streets() {
    for (StreetSegmentIdx segment_index = 0; segment_index < getNumStreetSegments(); ++segment_index ) {
        StreetSegmentInfo segment_info = getStreetSegmentInfo(segment_index);
        intersections_of_streets[segment_info.streetID].push_back(segment_info.from);
        intersections_of_streets[segment_info.streetID].push_back(segment_info.to);
    }

    for (int i = 0; i < intersections_of_streets.size(); ++i) {
        remove_duplicates (intersections_of_streets[i]);
    }

}

// create two vectors of int
// std::vector<double> street_segment_length;
// std::vector<float> street_segment_time;
// stores the length and travel_time of each segment
void load_street_segment_length() {
    street_segment_length.resize(getNumStreetSegments());
    street_segment_time.resize(getNumStreetSegments());
    for (StreetSegmentIdx street_segment_id = 0; street_segment_id < getNumStreetSegments(); ++street_segment_id) {
        StreetSegmentInfo bin = getStreetSegmentInfo(street_segment_id);
        float len = 0;
        float time = 0;
        LatLon p1;
        LatLon p2;

        int num_points = bin.numCurvePoints;
        IntersectionIdx From = bin.from;
        IntersectionIdx To = bin.to;


        if (num_points == 0) {
            p1 = getIntersectionPosition(From);
            p2 = getIntersectionPosition(To);
            len = findDistanceBetweenTwoPoints(p1, p2);
            if (bin.speedLimit == 0) {
                time = INT_MAX;
            }
            time = len / bin.speedLimit;
            street_segment_length[street_segment_id] = len;
            street_segment_time[street_segment_id] = time;
            continue;
        }

        p1 = getIntersectionPosition(From);
        p2 = getStreetSegmentCurvePoint (street_segment_id, 0);
        len += findDistanceBetweenTwoPoints(p1, p2);

        for (int i = 0; i < (num_points - 1); ++i ) {
            p1 = getStreetSegmentCurvePoint (street_segment_id, i);
            p2 = getStreetSegmentCurvePoint (street_segment_id, i + 1);
            len += findDistanceBetweenTwoPoints(p1, p2);
        }

        p1 = getStreetSegmentCurvePoint (street_segment_id, (num_points-1));
        p2 = getIntersectionPosition(To);
        len += findDistanceBetweenTwoPoints(p1, p2);
        if (bin.speedLimit == 0) {
            time = INT_MAX;
        }
        time = len / bin.speedLimit;
        street_segment_length[street_segment_id] = len;
        street_segment_time[street_segment_id] = time;
        continue;
    } 
}


// -------------------------Milestone 2-------------------------------------------------------

// load vector map_bounds
// 0: max_lat
// 1: min_lat
// 2: max_lon
// 3: min_lon
// 4: avg_lat
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
void load_intersections() {
    double max_lat = getIntersectionPosition(0).latitude();
    double min_lat = max_lat;
    double max_lon = getIntersectionPosition(0).longitude();
    double min_lon = max_lon;
    intersections.resize(getNumIntersections());

    // Preload intersections data and calculate the map_bounds
    for (int id = 0; id < getNumIntersections(); ++id) {
        // Preload the data
        intersections[id].position = getIntersectionPosition(id);
        intersections[id].name = getIntersectionName(id);

        max_lat = std::max(max_lat, intersections[id].position.latitude());
        min_lat = std::min(min_lat, intersections[id].position.latitude());
        max_lon = std::max(max_lon, intersections[id].position.longitude());
        min_lon = std::min(min_lon, intersections[id].position.longitude());
    }

    // prestore the min/max latlon
    map_bounds[0] = max_lat;
    map_bounds[1] = min_lat;
    map_bounds[2] = max_lon;
    map_bounds[3] = min_lon;
    avg_lat_global = (max_lat + min_lat) / 2 * kDegreeToRadian;
    xy_bounds[0] = x_from_lon(max_lon);
    xy_bounds[1] = x_from_lon(min_lon);
    xy_bounds[2] = y_from_lat(max_lat);
    xy_bounds[3] = y_from_lat(min_lat);
     
    // preload the intersections.xy_loc
    for (int id = 0; id < getNumIntersections(); ++id) {
        float x = x_from_lon(intersections[id].position.longitude());
        float y = y_from_lat(intersections[id].position.latitude());
        ezgl::point2d points{x, y};
        intersections[id].xy_loc = points;
        intersections[id].highlight = false;
    }

    return;
}

void load_osm_way_map() {
    for (int osm_id = 0; osm_id < getNumberOfWays(); osm_id++) {
        const OSMWay* osm_way = getWayByIndex(osm_id);
        osm_way_map.insert({(osm_way -> id()), osm_way});
    }
}




void load_segments() {
    segments.resize(getNumStreetSegments());
    
    // Preload middle of streets
    // int text_per_half_street = 1;
    // street_points.resize(getNumStreets() * text_per_half_street);

    for (StreetIdx id = 0; id < getNumStreets(); ++id) {
        // Get extremeties of each street
        StreetSegmentIdx first_street_segment = (segments_of_streets[id])[0]; // Use of another global variable "segments_of_streets"
        StreetSegmentIdx last_street_segment = (segments_of_streets[id])[segments_of_streets[id].size() - 1];

        // Get outer extremity of each street segment
        IntersectionIdx street_start = getStreetSegmentInfo(first_street_segment).from;
        IntersectionIdx street_end = getStreetSegmentInfo(last_street_segment).to;

        float x_start = x_from_lon(getIntersectionPosition(street_start).longitude());
        float y_start = y_from_lat(getIntersectionPosition(street_start).latitude());

        float x_end = x_from_lon(getIntersectionPosition(street_end).longitude());
        float y_end = y_from_lat(getIntersectionPosition(street_end).latitude());

        float x_middle = (x_start + x_end) / 2;
        float y_middle = (y_start + y_end) / 2;

        ezgl::point2d middle_point {x_middle, y_middle};
        ezgl::point2d start_point {x_start, y_start};
        ezgl::point2d end_point {x_end, y_end};
        std::pair<ezgl::point2d, ezgl::point2d> coordinates(start_point, end_point);

        street_points[id] = coordinates; // Same # of street ids as coordinates        
    }

    // Street segment information initializers
    IntersectionIdx origin;
    IntersectionIdx destination;
    int num_curve_points;
    StreetIdx seg_street_id;
    bool seg_one_way;
    float seg_speed_lim;
    float seg_length;
    double seg_angle;
    bool is_major;
    std::string street_type;


    // Pre declaring all variables that are going to be frequently used in the following loading
    float x_origin, y_origin, x_destination, y_destination, x_avg, y_avg, x_curve, y_curve;
    ezgl::point2d origin_point, destination_point, avg_point, curve_point;
    StreetSegmentInfo segment_info;

    for (StreetSegmentIdx id = 0; id < getNumStreetSegments(); ++id) {
        segment_info = getStreetSegmentInfo(id);
        
        // Get/set street segment information
        origin = segment_info.from;
        destination = segment_info.to;
        num_curve_points = segment_info.numCurvePoints;
        seg_street_id = segment_info.streetID;
        seg_one_way = segment_info.oneWay;
        seg_speed_lim = segment_info.speedLimit;
        is_major = (segment_info.speedLimit >= 27.778); 

        OSMID source_way_osmid = segment_info.wayOSMID;
        
        street_type = osmnode_values[source_way_osmid];

        // Find matching street type
        if (osmnode_values.count(source_way_osmid) > 0) {
            street_type = osmnode_values[source_way_osmid];
        } else {
            std::cout << "Key not found in map" << std::endl;
        }

        segments[id].num_curve_p = num_curve_points;
        segments[id].street_id = seg_street_id;
        segments[id].street_name = getStreetName(seg_street_id);
        segments[id].road_type = street_type;
        segments[id].one_way = seg_one_way;
        segments[id].speed_limit = seg_speed_lim;
        segments[id].major_road = is_major;

        // Preload origin/destination points and middle points
        x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
        y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
        origin_point.x = x_origin;
        origin_point.y = y_origin;
        segments[id].from_xy = origin_point;

        x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
        y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
        destination_point.x = x_destination;
        destination_point.y = y_destination;
        segments[id].to_xy = destination_point;

        x_avg = (x_origin + x_destination) / 2;
        y_avg = (y_origin + y_destination) / 2;
        avg_point.x = x_avg;
        avg_point.y = y_avg;
        segments[id].middle_xy = avg_point;

        seg_length = sqrt(((x_destination - x_origin) * (x_destination - x_origin)) + ((y_destination - y_origin) * (y_destination - y_origin)));
        segments[id].length = seg_length;
        seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
        segments[id].angle = seg_angle;

        // Preload all curve points into vector
        (segments[id].seg_curve_points).resize(num_curve_points);

        for (int j = 0; j < num_curve_points; ++j) {
            x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
            y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
            curve_point.x = x_curve;
            curve_point.y = y_curve;
            segments[id].seg_curve_points[j] = curve_point;
        }

        // -----------------------Zoom set small-----------------------
        if ((street_type == "track") || (street_type == "footway") || (street_type == "bridleway") || (street_type == "path")) {
            street_segment_data cur_segment_small;
            segment_info = getStreetSegmentInfo(id);
            
            // Get/set street segment information
            origin = segment_info.from;
            destination = segment_info.to;
            num_curve_points = segment_info.numCurvePoints;
            seg_street_id = segment_info.streetID;
            seg_one_way = segment_info.oneWay;
            seg_speed_lim = segment_info.speedLimit;
            is_major = (segment_info.speedLimit >= 27.778); 

            cur_segment_small.num_curve_p = num_curve_points;
            cur_segment_small.street_id = seg_street_id;
            cur_segment_small.street_name = getStreetName(seg_street_id);
            cur_segment_small.one_way = seg_one_way;
            cur_segment_small.speed_limit = seg_speed_lim;
            cur_segment_small.major_road = is_major; 

            x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
            y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
            origin_point.x = x_origin;
            origin_point.y = y_origin;
            cur_segment_small.from_xy = origin_point;

            x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
            y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
            destination_point.x = x_destination;
            destination_point.y = y_destination;
            cur_segment_small.to_xy = destination_point;

            x_avg = (x_origin + x_destination) / 2;
            y_avg = (y_origin + y_destination) / 2;
            avg_point.x = x_avg;
            avg_point.y = y_avg;
            cur_segment_small.middle_xy = avg_point;

            seg_length = street_segment_length[id];
            cur_segment_small.length = seg_length;
            seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
            cur_segment_small.angle = seg_angle;

            street_id_small.insert(seg_street_id);

            // Preload all curve points into vector
            (cur_segment_small.seg_curve_points).resize(num_curve_points);

            for (int j = 0; j < num_curve_points; ++j) {
                x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
                y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
                curve_point.x = x_curve;
                curve_point.y = y_curve;
                cur_segment_small.seg_curve_points[j] = curve_point;
            }

            segments_zoom_small.push_back(cur_segment_small);
            
        }

        // -----------------------Zoom set 0-----------------------
        if ((street_type == "unclassified") || (street_type == "residential") || (street_type == "living_street") || (street_type == "service")) {
            street_segment_data cur_segment_0;
            segment_info = getStreetSegmentInfo(id);
            
        
            // Get/set street segment information
            origin = segment_info.from;
            destination = segment_info.to;
            num_curve_points = segment_info.numCurvePoints;
            seg_street_id = segment_info.streetID;
            seg_one_way = segment_info.oneWay;
            seg_speed_lim = segment_info.speedLimit;
            is_major = (segment_info.speedLimit >= 27.778); 

            cur_segment_0.num_curve_p = num_curve_points;
            cur_segment_0.street_id = seg_street_id;
            cur_segment_0.street_name = getStreetName(seg_street_id);
            cur_segment_0.one_way = seg_one_way;
            cur_segment_0.speed_limit = seg_speed_lim;
            cur_segment_0.major_road = is_major; 

            x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
            y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
            origin_point.x = x_origin;
            origin_point.y = y_origin;
            cur_segment_0.from_xy = origin_point;

            x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
            y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
            destination_point.x = x_destination;
            destination_point.y = y_destination;
            cur_segment_0.to_xy = destination_point;

            x_avg = (x_origin + x_destination) / 2;
            y_avg = (y_origin + y_destination) / 2;
            avg_point.x = x_avg;
            avg_point.y = y_avg;
            cur_segment_0.middle_xy = avg_point;

            seg_length = street_segment_length[id];
            cur_segment_0.length = seg_length;
            seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
            cur_segment_0.angle = seg_angle;

            street_id_0.insert(seg_street_id);

            // Preload all curve points into vector
            (cur_segment_0.seg_curve_points).resize(num_curve_points);

            for (int j = 0; j < num_curve_points; ++j) {
                x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
                y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
                curve_point.x = x_curve;
                curve_point.y = y_curve;
                cur_segment_0.seg_curve_points[j] = curve_point;
            }

            segments_zoom_0.push_back(cur_segment_0);
            
        }

        // -----------------------Zoom set 1-----------------------
        if ((street_type == "tertiary") || (street_type == "tertiary_link")) {
            street_segment_data cur_segment_1;
            segment_info = getStreetSegmentInfo(id);
            
            // Get/set street segment information
            origin = segment_info.from;
            destination = segment_info.to;
            num_curve_points = segment_info.numCurvePoints;
            seg_street_id = segment_info.streetID;
            seg_one_way = segment_info.oneWay;
            seg_speed_lim = segment_info.speedLimit;
            is_major = (segment_info.speedLimit >= 27.778);

            cur_segment_1.num_curve_p = num_curve_points;
            cur_segment_1.street_id = seg_street_id;
            cur_segment_1.street_name = getStreetName(seg_street_id);
            cur_segment_1.one_way = seg_one_way;
            cur_segment_1.speed_limit = seg_speed_lim;
            cur_segment_1.major_road = is_major;

            x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
            y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
            origin_point.x = x_origin;
            origin_point.y = y_origin;
            cur_segment_1.from_xy = origin_point;

            x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
            y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
            destination_point.x = x_destination;
            destination_point.y = y_destination;
            cur_segment_1.to_xy = destination_point;

            x_avg = (x_origin + x_destination) / 2;
            y_avg = (y_origin + y_destination) / 2;
            avg_point.x = x_avg;
            avg_point.y = y_avg;
            cur_segment_1.middle_xy = avg_point;

            seg_length = street_segment_length[id];
            cur_segment_1.length = seg_length;
            seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
            cur_segment_1.angle = seg_angle;

            street_id_1.insert(seg_street_id);

            // Preload all curve points into vector
            (cur_segment_1.seg_curve_points).resize(num_curve_points);

            for (int j = 0; j < num_curve_points; ++j) {
                x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
                y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
                curve_point.x = x_curve;
                curve_point.y = y_curve;
                cur_segment_1.seg_curve_points[j] = curve_point;
            }

            segments_zoom_1.push_back(cur_segment_1);

        } 

        // -----------------------Zoom set 2-----------------------
        if ((street_type == "secondary") || (street_type == "secondary_link")) {
            street_segment_data cur_segment_2;
            segment_info = getStreetSegmentInfo(id);
            
            // Get/set street segment information
            origin = segment_info.from;
            destination = segment_info.to;
            num_curve_points = segment_info.numCurvePoints;
            seg_street_id = segment_info.streetID;
            seg_one_way = segment_info.oneWay;
            seg_speed_lim = segment_info.speedLimit;
            is_major = (segment_info.speedLimit >= 27.778);

            cur_segment_2.num_curve_p = num_curve_points;
            cur_segment_2.street_id = seg_street_id;
            cur_segment_2.street_name = getStreetName(seg_street_id);
            cur_segment_2.one_way = seg_one_way;
            cur_segment_2.speed_limit = seg_speed_lim;
            cur_segment_2.major_road = is_major;

            x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
            y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
            origin_point.x = x_origin;
            origin_point.y = y_origin;
            cur_segment_2.from_xy = origin_point;

            x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
            y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
            destination_point.x = x_destination;
            destination_point.y = y_destination;
            cur_segment_2.to_xy = destination_point;

            x_avg = (x_origin + x_destination) / 2;
            y_avg = (y_origin + y_destination) / 2;
            avg_point.x = x_avg;
            avg_point.y = y_avg;
            cur_segment_2.middle_xy = avg_point;

            seg_length = street_segment_length[id];
            cur_segment_2.length = seg_length;
            seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
            cur_segment_2.angle = seg_angle;

            street_id_2.insert(seg_street_id);

            // Preload all curve points into vector
            (cur_segment_2.seg_curve_points).resize(num_curve_points);

            for (int j = 0; j < num_curve_points; ++j) {
                x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
                y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
                curve_point.x = x_curve;
                curve_point.y = y_curve;
                cur_segment_2.seg_curve_points[j] = curve_point;
            }

            segments_zoom_2.push_back(cur_segment_2);

        }

        // -----------------------Zoom set 3-----------------------
        if ((street_type == "primary") || (street_type == "primary_link")) {
            street_segment_data cur_segment_3;
            segment_info = getStreetSegmentInfo(id);
            
            // Get/set street segment information
            origin = segment_info.from;
            destination = segment_info.to;
            num_curve_points = segment_info.numCurvePoints;
            seg_street_id = segment_info.streetID;
            seg_one_way = segment_info.oneWay;
            seg_speed_lim = segment_info.speedLimit;
            is_major = (segment_info.speedLimit >= 27.778);

            cur_segment_3.num_curve_p = num_curve_points;
            cur_segment_3.street_id = seg_street_id;
            cur_segment_3.street_name = getStreetName(seg_street_id);
            cur_segment_3.one_way = seg_one_way;
            cur_segment_3.speed_limit = seg_speed_lim;
            cur_segment_3.major_road = is_major;

            x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
            y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
            origin_point.x = x_origin;
            origin_point.y = y_origin;
            cur_segment_3.from_xy = origin_point;

            x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
            y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
            destination_point.x = x_destination;
            destination_point.y = y_destination;
            cur_segment_3.to_xy = destination_point;

            x_avg = (x_origin + x_destination) / 2;
            y_avg = (y_origin + y_destination) / 2;
            avg_point.x = x_avg;
            avg_point.y = y_avg;
            cur_segment_3.middle_xy = avg_point;

            seg_length = street_segment_length[id];
            cur_segment_3.length = seg_length;
            seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
            cur_segment_3.angle = seg_angle;

            street_id_3.insert(seg_street_id);

            // Preload all curve points into vector
            (cur_segment_3.seg_curve_points).resize(num_curve_points);

            for (int j = 0; j < num_curve_points; ++j) {
                x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
                y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
                curve_point.x = x_curve;
                curve_point.y = y_curve;
                cur_segment_3.seg_curve_points[j] = curve_point;
            }

            segments_zoom_3.push_back(cur_segment_3);

        }

        // -----------------------Zoom set 4-----------------------
        if ((street_type == "motorway") || (street_type == "trunk") || (street_type == "motorway_link") || (street_type == "trunk_link")) {
            street_segment_data cur_segment_4;
            segment_info = getStreetSegmentInfo(id);
            
            // Get/set street segment information
            origin = segment_info.from;
            destination = segment_info.to;
            num_curve_points = segment_info.numCurvePoints;
            seg_street_id = segment_info.streetID;
            seg_one_way = segment_info.oneWay;
            seg_speed_lim = segment_info.speedLimit;
            is_major = (segment_info.speedLimit >= 27.778);
            
            cur_segment_4.num_curve_p = num_curve_points;
            cur_segment_4.street_id = seg_street_id;
            cur_segment_4.street_name = getStreetName(seg_street_id);
            cur_segment_4.one_way = seg_one_way;
            cur_segment_4.speed_limit = seg_speed_lim;
            cur_segment_4.major_road = is_major; 

            x_origin = x_from_lon(getIntersectionPosition(origin).longitude());
            y_origin = y_from_lat(getIntersectionPosition(origin).latitude());
            origin_point.x = x_origin;
            origin_point.y = y_origin;
            cur_segment_4.from_xy = origin_point;

            x_destination = x_from_lon(getIntersectionPosition(destination).longitude());
            y_destination = y_from_lat(getIntersectionPosition(destination).latitude());
            destination_point.x = x_destination;
            destination_point.y = y_destination;
            cur_segment_4.to_xy = destination_point;

            x_avg = (x_origin + x_destination) / 2;
            y_avg = (y_origin + y_destination) / 2;
            avg_point.x = x_avg;
            avg_point.y = y_avg;
            cur_segment_4.middle_xy = avg_point;

            seg_length = street_segment_length[id];
            cur_segment_4.length = seg_length;
            seg_angle = std::atan((y_destination - y_origin) / (x_destination - x_origin));
            cur_segment_4.angle = seg_angle;

            street_id_4.insert(seg_street_id);

            // Preload all curve points into vector
            (cur_segment_4.seg_curve_points).resize(num_curve_points);

            for (int j = 0; j < num_curve_points; ++j) {
                x_curve = x_from_lon(getStreetSegmentCurvePoint(id, j).longitude());
                y_curve = y_from_lat(getStreetSegmentCurvePoint(id, j).latitude());
                curve_point.x = x_curve;
                curve_point.y = y_curve;
                cur_segment_4.seg_curve_points[j] = curve_point;
            }

            segments_zoom_4.push_back(cur_segment_4);
        }
            
                
    }

    num_zoom_small = segments_zoom_small.size();
    num_zoom_0 = segments_zoom_0.size();
    num_zoom_1 = segments_zoom_1.size();
    num_zoom_2 = segments_zoom_2.size();
    num_zoom_3 = segments_zoom_3.size();
    num_zoom_4 = segments_zoom_4.size();
}

void load_pois() {
    // POI information initializers
    std::string poi_type;
    std::string poi_name;

    for (POIIdx id = 0; id < getNumPointsOfInterest(); ++id) {
        poi_type = getPOIType(id);

        // -----------------------Zoom set small-----------------------
        if ((poi_type == "community_centre") || (poi_type == "theatre") || (poi_type == "cinema") || (poi_type == "bank") || (poi_type == "police") || (poi_type == "dentist") || (poi_type == "bicycle_parking") || (poi_type == "fuel") || (poi_type == "cafe") || (poi_type == "parking") || (poi_type == "post_office") || (poi_type == "restaurant")) {
            poi_data cur_poi_small;

            // Get/set POI information
            poi_name = getPOIName(id);
            cur_poi_small.type = poi_type;
            cur_poi_small.name = poi_name;

            float x_coord = x_from_lon(getPOIPosition(id).longitude());
            float y_coord = y_from_lat(getPOIPosition(id).latitude());
            ezgl::point2d poi_point{x_coord, y_coord};
            cur_poi_small.coord_xy = poi_point;

            poi_zoom_small.push_back(cur_poi_small);
        }

        // -----------------------Zoom set 0-----------------------
        if ((poi_type == "prison") || (poi_type == "casino") || (poi_type == "college") || (poi_type == "school") || (poi_type == "place_of_worship") || (poi_type == "library")) {
            poi_data cur_poi_0;

            // Get/set POI information
            poi_name = getPOIName(id);
            cur_poi_0.type = poi_type;
            cur_poi_0.name = poi_name;

            float x_coord = x_from_lon(getPOIPosition(id).longitude());
            float y_coord = y_from_lat(getPOIPosition(id).latitude());
            ezgl::point2d poi_point{x_coord, y_coord};
            cur_poi_0.coord_xy = poi_point;

            poi_zoom_0.push_back(cur_poi_0);
        }

        // -----------------------Zoom set 1-----------------------
        if ((poi_type == "hospital")) {
            poi_data cur_poi_1;

            // Get/set POI information
            poi_name = getPOIName(id);
            cur_poi_1.type = poi_type;
            cur_poi_1.name = poi_name;

            float x_coord = x_from_lon(getPOIPosition(id).longitude());
            float y_coord = y_from_lat(getPOIPosition(id).latitude());
            ezgl::point2d poi_point{x_coord, y_coord};
            cur_poi_1.coord_xy = poi_point;

            poi_zoom_1.push_back(cur_poi_1);
        }

        // -----------------------Zoom set 2-----------------------
        // None

        // -----------------------Zoom set 3-----------------------
        // None

        // -----------------------Zoom set 4-----------------------
        // None
    }
}

void load_osm_ways() {
    std::string key = "highway";

    for (int way_id = 0; way_id < getNumberOfWays(); ++way_id) { // Iterate through all ways
        const OSMWay* cur_way = getWayByIndex(way_id); // Get current way
        for (int tag = 0; tag < getTagCount(cur_way); ++tag) { // Iterate through the tags of the current way
            std::string way_key, way_value;
            std::tie(way_key, way_value) = getTagPair(cur_way, tag);
            
            if (way_key == key) { // If road found, get value and way members
                osmnode_values[cur_way -> id()] = way_value;
            }
        }
    }
}

void load_subway_stations() {
    std::string key = "station";
    std::string value = "subway";

    for (int node_id = 0; node_id < getNumberOfNodes(); ++node_id) { 
        const OSMNode* cur_node = getNodeByIndex(node_id); // Get current node
        for (int tag = 0; tag < getTagCount(cur_node); ++tag) { // Iterate through the tags of the current node
            std::pair<std::string, std::string> tag_pair = getTagPair(cur_node, tag);
            // search for subway stations
            if (tag_pair.first == key && tag_pair.second == value) {
                subway_osm_nodes.push_back(cur_node);
            }
        }
    }

    for (int i = 0; i < subway_osm_nodes.size(); ++i) {
        subway_info temp_info;
        for (unsigned j = 0; j < getTagCount(subway_osm_nodes[i]); j++) {
            
            std::pair<std::string, std::string> tagPair = getTagPair(subway_osm_nodes[i], j);
            LatLon temp_loc = getNodeCoords(subway_osm_nodes[i]);
            temp_info.latlon_loc = temp_loc;
            double temp_x = x_from_lon(temp_loc.longitude());
            double temp_y = y_from_lat(temp_loc.latitude());
            temp_info.xy_pos.x = temp_x;
            temp_info.xy_pos.y = temp_y;

            // get the name tag
            if (tagPair.first == "name") {
                temp_info.subway_name = tagPair.second;
                std::pair<const OSMNode*, subway_info> temp_subway_node (subway_osm_nodes[i], temp_info);
                subway_node_info.insert(temp_subway_node);                
                break; 
            }

        }
    }
}

/*
typedef struct {
   std::string feature_name;
   FeatureType feature_type;
   int feature_num_curve_points;
   std::vector<ezgl::point2d> feature_curve_points;
} feature_data;
extern std::vector<feature_data> features;
*/
void load_features() {
    features.resize(getNumFeatures());
    for (FeatureIdx id = 0; id < getNumFeatures(); ++id) {
        features[id].feature_name = getFeatureName(id);
        features[id].feature_type = getFeatureType(id);
        features[id].feature_id = id;
        features[id].feature_area = findFeatureArea(id);

        int num_feature_points = getNumFeaturePoints(id);
        features[id].feature_num_curve_points = num_feature_points;

        features[id].feature_curve_points.resize(num_feature_points);
        for (int j = 0; j < num_feature_points; ++j) {
            LatLon latlon_temp = getFeaturePoint(id, j);
            float x_temp = x_from_lon(latlon_temp.longitude());
            float y_temp = y_from_lat(latlon_temp.latitude());
            ezgl::point2d temp_point{x_temp, y_temp};
            features[id].feature_curve_points[j] = temp_point;
        }

        if (getFeatureType(id) == PARK) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_park.push_back(cur_feature);

        } if (getFeatureType(id) == BEACH) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_beach.push_back(cur_feature);

        } if (getFeatureType(id) == LAKE) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_lake.push_back(cur_feature);

        } if (getFeatureType(id) == RIVER) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_river.push_back(cur_feature);

        } if (getFeatureType(id) == ISLAND) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_island.push_back(cur_feature);

        } if (getFeatureType(id) == BUILDING) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_building.push_back(cur_feature);

        } if (getFeatureType(id) == GREENSPACE) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_greenspace.push_back(cur_feature);

        } if (getFeatureType(id) == GOLFCOURSE) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_golfcourse.push_back(cur_feature);

        } if (getFeatureType(id) == STREAM) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_stream.push_back(cur_feature);

        } if (getFeatureType(id) == GLACIER) {
            feature_data cur_feature;
            cur_feature.feature_name = getFeatureName(id);
            cur_feature.feature_type = getFeatureType(id);
            cur_feature.feature_id = id;
            cur_feature.feature_area = findFeatureArea(id);

            num_feature_points = getNumFeaturePoints(id);
            cur_feature.feature_num_curve_points = num_feature_points;

            cur_feature.feature_curve_points.resize(num_feature_points);
            for (int j = 0; j < num_feature_points; ++j) {
                LatLon latlon_temp = getFeaturePoint(id, j);
                float x_temp = x_from_lon(latlon_temp.longitude());
                float y_temp = y_from_lat(latlon_temp.latitude());
                ezgl::point2d temp_point{x_temp, y_temp};
                cur_feature.feature_curve_points[j] = temp_point;
            }

            features_glacier.push_back(cur_feature);
        } 
    }

    return;
}


// loading the following data into a vector:
/*
typedef struct {
   std::string POItype;
   std::string POIname;
   ezgl::point2d POI_point;
} POI_data;
extern std::vector<POI_data> POIs;
*/
void load_POIs(){
    POIs.resize(getNumPointsOfInterest());
    for (POIIdx id = 0; id<getNumPointsOfInterest(); ++id){
        POIs[id].POItype = getPOIType(id);
        POIs[id].POIname = getPOIName(id);
        double x = x_from_lon(getPOIPosition(id).longitude());
        double y = y_from_lat(getPOIPosition(id).latitude());
        ezgl::point2d point{x, y};
        POIs[id].POI_point = point;
    }
}

// -------------------------Milestone 3-------------------------------------------------------


// Load the nodes used in m3

void load_nodes() {
    nodes.resize(getNumIntersections());
    for (int i = 0; i < getNumIntersections(); ++i) {
        nodes[i].nodeID = i;
        nodes[i].edges.resize(getNumIntersectionStreetSegment(i));
        for (int j = 0; j < getNumIntersectionStreetSegment(i); ++j) {
            nodes[i].edges[j] = getIntersectionStreetSegment(i, j); 
        }

        nodes[i].reachingEdge = -1;
        nodes[i].visited = false;
        nodes[i].bestTime = std::numeric_limits<double>::infinity();  
        // std::cout << "NodeID: " << nodes[i].nodeID << std::endl;
        
        // std::cout << "Reaching edge: " << nodes[i].reachingEdge << std::endl;
        // std::cout << "Visited: " << nodes[i].visited << std::endl;
    }
}
// -------------------------Helper functions ended--------------------------------------------

