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

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <climits>
#include <limits>
#include <cmath>
#include <set>
#include <string>
#include "m2_helper.h"
#include "globals.h"
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <list>
#include "globals.h"
#include <iomanip>
#include <sstream>



// Functions


// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0. The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path. If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty) {
    double travel_time = 0;

    // Check for empty vector
    if (path.size() == 0) {
        return travel_time;
    }

    for (int seg = 0; seg < path.size(); ++seg) {
        // Find segment travel time
        double seg_time = street_segment_time[path[seg]];
        travel_time += seg_time;

        // Check for turns
        if (seg != path.size() - 1) {
            if (getStreetSegmentInfo(path[seg]).streetID != getStreetSegmentInfo(path[seg + 1]).streetID) { // Compare current segment with next segment
                travel_time += turn_penalty;
            }
        }
    }    

    return travel_time;
}



// Helper function to generate a set of instructions in string
std::vector<std::string> generatePathDirections(std::list<Path> path);
double angleFromLatLon(LatLon start, LatLon end);

// Read two street segments from and to (Presumes that they are intersected)
// Figure out whether turning left or right to turn from "from" to "to"
// Return true if turn left
bool turn_left(StreetSegmentIdx from, StreetSegmentIdx to);

// Round the distance to its closest 10
int round_distance(double n);

std::string double_to_string(double value, int precision);

// Full implementation of Dijkstra's algorithm

std::vector<StreetSegmentIdx> bfsTraceBack(IntersectionIdx end_intersection, IntersectionIdx start_intersection);

// struct WaveElem {
//     IntersectionIdx nodeID;
//     StreetSegmentIdx edgeID; // edge used to reach this node 
//     double travelTime;       // Total travel time to reach node
//     WaveElem (int n, int e, float time) {
//     nodeID = n; edgeID = e; travelTime = time; }
// };

// // Used for the priority queue
// struct CompareWaveElem {
//     bool operator()(const WaveElem& we1, const WaveElem& we2) {
//         return we1.travelTime > we2.travelTime;
//     }
// };

// Helpful declaration of the struct storing baacktracking information
// for the Dijkstra's algorithm
// Struct declared in globals.h
/* 
typedef struct {
    IntersectionIdx nodeID;
    std::vector<StreetSegmentIdx> edges;
        // Outgoing edges etc.
    StreetSegmentIdx reachingEdge; // edge used to reach node double bestTime; // Shortest time found to this node so far
    bool visited;
    double bestTime;
} Node;

*/

#define NO_EDGE -1  // Illegal edge ID -> no edge



// BFS search
// Tracing back to get the shortest path from start_intersection to end_intersection
// Returns the vector of all Street Segments that form this path
std::vector<StreetSegmentIdx> bfsTraceBack(IntersectionIdx end_intersection, IntersectionIdx start_intersection) {
    std::list<Path> path;

    // std::list<StreetSegmentIdx> path;
    IntersectionIdx currNodeID = end_intersection;
    StreetSegmentIdx prevEdge = nodes[currNodeID].reachingEdge;

    while (prevEdge != NO_EDGE) {
        // path.push_front(prevEdge);
        Path past_path;
        past_path.passing_edge = prevEdge;
 
        past_path.to_node = currNodeID;

        StreetSegmentInfo segment_info = getStreetSegmentInfo(prevEdge);
        if (segment_info.from == currNodeID) {
            currNodeID = segment_info.to;
        } else {
            currNodeID = segment_info.from;
        }
        past_path.from_node = currNodeID;
        path.push_front(past_path);

        if (currNodeID == start_intersection) {
            break; // reached the start intersection
        }

        prevEdge = nodes[currNodeID].reachingEdge;

        if (prevEdge == NO_EDGE) {
            path.clear(); // no reaching edge, return an empty path
            break;
        }
    }

    trip_travel_time = 0;

    // The return vector for passing the autotester:
    std::vector<StreetSegmentIdx> path_vector;
    for (auto it : path) {
        path_vector.push_back(it.passing_edge);
        trip_travel_time += segments[it.passing_edge].length / segments[it.passing_edge].speed_limit;
        
    }

    // Set global path variable
    found_path_global.draw_found_path = true;
    found_path_global.found_path = path_vector;

    generatePathDirections(path);

    return path_vector;
}




// Returns a path (route) between the start intersection (intersect_id.first)
// and the destination intersection (intersect_id.second), if one exists.
// This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds). If no path exists, this routine
// returns an empty (size == 0) vector. If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids, const double turn_penalty) {
    IntersectionIdx start_intersection = intersect_ids.first;
    IntersectionIdx end_intersection = intersect_ids.second;

    if (start_intersection == end_intersection) {
        std::vector<StreetSegmentIdx> empty;
        return empty;
    }

    // Graph layout:
    // Node - intersection
    // Edge - street segments

    // Initialize the node vector every time
    nodes.resize(getNumIntersections());
    for (IntersectionIdx i = 0; i < getNumIntersections(); ++i) {
        nodes[i].nodeID = i;
        nodes[i].reachingEdge = NO_EDGE;
        nodes[i].visited = false;
        nodes[i].bestTime = std::numeric_limits<double>::infinity();
    }

    // std::list<WaveElem> wavefront;
    std::priority_queue<WaveElem, std::vector<WaveElem>, CompareWaveElem> pq;
    // wavefront.push_back (WaveElem (start_intersection, NO_EDGE, 0));
    pq.push(WaveElem (start_intersection, NO_EDGE, 0));
    nodes[start_intersection].bestTime = 0;

    // StreetSegmentIdx previousEdge = NO_EDGE;
    // account for one way
    // account for turning
    while (!pq.empty()) {
        // Get the node with the smallest bestTime in the wavefront

        WaveElem curr = pq.top();
        pq.pop();

        int currID = curr.nodeID;

        if (nodes[currID].visited) { // Only visit once
            continue;
        }

        if (curr.travelTime > nodes[currID].bestTime) {
            continue;
        }

        // Was this a better path to this node? Update if so
        nodes[currID].reachingEdge = curr.edgeID; 
        nodes[currID].bestTime = curr.travelTime;
        nodes[currID].visited = true;

        if (currID == end_intersection) {
            return bfsTraceBack(end_intersection, start_intersection);
        }

        // each outEdge of curr.nodeID
        for (int i = 0; i < (nodes[currID].edges).size(); ++i) {
            StreetSegmentIdx outEdgeID = nodes[currID].edges[i];
            StreetSegmentInfo outEdge = getStreetSegmentInfo(outEdgeID);
            int toNodeID;

            if (outEdge.oneWay && outEdge.to == currID) {
                continue;      
            }

            if (outEdge.from == nodes[currID].nodeID) {
                toNodeID = outEdge.to;
            }
            else {
                toNodeID = outEdge.from;
            }

            // turn_penalty
            double total_time = nodes[currID].bestTime + findStreetSegmentTravelTime(outEdgeID);
            if (curr.edgeID != NO_EDGE) {
                // Account for turn_penalty if turning from the previous edge
                StreetSegmentInfo previousEdge = getStreetSegmentInfo(curr.edgeID);
                if (previousEdge.streetID != outEdge.streetID) {
                    total_time += turn_penalty;
                }
            }   
            

            pq.push(WaveElem(toNodeID, outEdgeID, total_time));
        }
    }
    
    // No path exists
    std::vector<StreetSegmentIdx> empty;
    return empty;

}


// Helper function to generate a set of instructions in string
std::vector<std::string> generatePathDirections(std::list<Path> path) {

    std::vector<Path> path_v(path.begin(), path.end());


    instructions.clear();
    std::string turn_direction = "left";


    // Compute the direction of the first edge
    LatLon p1 = getIntersectionPosition(path_v[0].from_node);
    LatLon p2 = getIntersectionPosition(path_v[0].to_node);
    double angle = angleFromLatLon(p1, p2);

    // directions:
    std::string direction;
    std::string inst;

    if ((angle > 45 || angle == 45) && angle < 135) {
        direction = "north";
    }
    else if ((angle > 135 || angle == 135) || angle < -135) {
        direction = "west";
    }
    else if ((angle > -135 || angle == -135) && angle < -45) {
        direction = "south";
    }
    else if ((angle > -45 || angle == -45) && angle < 45) {
        direction = "east";
    }

    double total_distance = 0;
    double total_time = 0;
    std::string double_string = "";

    std::string i1 = "Starting from " + getIntersectionName(path_v[0].from_node) + ".\n";
    instructions.push_back(i1);
    bool first_instruction = true;

    double curr_street_distance = findStreetSegmentLength(path_v[0].passing_edge);
    total_distance += segments[path_v[0].passing_edge].length;
    total_time += segments[path_v[0].passing_edge].length / segments[path_v[0].passing_edge].speed_limit;

    // if only passes through one street segment
    if (path_v.size() == 1) {
        std::string last_street_name = getStreetName(getStreetSegmentInfo(path_v[0].passing_edge).streetID);
        if (curr_street_distance > 1000) {
            curr_street_distance = curr_street_distance / 1000.0;
            double_string = double_to_string(curr_street_distance, 1);
            inst = "Head " + direction + " on " + last_street_name + ". (" + double_string + " km)\n";

        }
        else {
            int int_distance = round_distance(curr_street_distance);
            inst = "Head " + direction + " on " + last_street_name + ". (" + std::to_string(int_distance) + " m)\n";        
        }

        instructions.push_back(inst);

    }


    for (int i = 1; i < path_v.size(); ++i) {
        std::string last_street_name = getStreetName(getStreetSegmentInfo(path_v[i - 1].passing_edge).streetID);
        std::string curr_street_name = getStreetName(getStreetSegmentInfo(path_v[i].passing_edge).streetID);
        total_distance += segments[path_v[i].passing_edge].length;
        total_time += segments[path_v[i].passing_edge].length / segments[path_v[i].passing_edge].speed_limit;

   
        // Just accumulate the travel distance if travelling on the same street
        if (getStreetSegmentInfo(path_v[i - 1].passing_edge).streetID == getStreetSegmentInfo(path_v[i].passing_edge).streetID) {
            curr_street_distance += findStreetSegmentLength(path_v[i].passing_edge);
        }

        // a turn happens
        // Wrap up the previous street 
        else {
            // figure out the turning direction
            // either left or right
            // Reaches the end of instruction
            if (first_instruction) {
                first_instruction = false;
                if (curr_street_distance > 1000) {
                    curr_street_distance = curr_street_distance / 1000.0;
                    double_string = double_to_string(curr_street_distance, 1);
                    inst = "Head " + direction + " on " + last_street_name + ". (" + double_string + " km)\n";
                }
                else {
                    int int_distance = round_distance(curr_street_distance);
                    inst = "Head " + direction + " on " + last_street_name + ". (" + std::to_string(int_distance) + " m)\n";                    
                }

                instructions.push_back(inst);

                curr_street_distance = findStreetSegmentLength(path_v[i].passing_edge);
                // update turn direction
                if (turn_left(path_v[i - 1].passing_edge, path_v[i].passing_edge)) {
                    turn_direction = "left";
                }
                else {
                    turn_direction = "right";
                }
                
            }
            else {
                // use the last turn direction
                if (curr_street_distance < 10) {
                    inst = "Immediately turn " + turn_direction + " onto " + last_street_name + ".\n";
                }
                else {
                    if (curr_street_distance > 1000) {
                        curr_street_distance = curr_street_distance / 1000.0;
                        double_string = double_to_string(curr_street_distance, 1);
                        inst = "Turn " + turn_direction + " onto " + last_street_name + ". (" + double_string + " km)\n";

                    }
                    else if (curr_street_distance < 10) {
                        inst = "Immediately turn " + turn_direction + " onto " + curr_street_name + ".\n";
                    }
                    else {
                        int int_distance = round_distance(curr_street_distance);
                        inst = "Turn " + turn_direction + " onto " + last_street_name + ". (" + std::to_string(int_distance) + " m)\n";                        
                    }

                }
                instructions.push_back(inst);

                curr_street_distance = findStreetSegmentLength(path_v[i].passing_edge);
                // update turn direction
                if (turn_left(path_v[i - 1].passing_edge, path_v[i].passing_edge)) {
                    turn_direction = "left";
                }
                else {
                    turn_direction = "right";
                }
            }
        }

        // When it reaches the final edge
        if (i == (path_v.size() - 1)) {
            if (first_instruction) {
                if (curr_street_distance > 1000) {
                    curr_street_distance = curr_street_distance / 1000.0;
                    double_string = double_to_string(curr_street_distance, 1);
                    inst = "Head " + direction + " on " + curr_street_name + ". (" + double_string + " km)\n";
                }
                else {
                    int int_distance = round_distance(curr_street_distance);
                    inst = "Head " + direction + " on " + curr_street_name + ". (" + std::to_string(int_distance) + " m)\n";
                }

                instructions.push_back(inst);
            }

            // Wrap up the current street
            else if (!first_instruction){
                if (curr_street_distance < 10) {
                    inst = "Immediately turn " + turn_direction + " onto " + curr_street_name + ".\n";
                }
                else {
                    if (curr_street_distance > 1000) {
                        curr_street_distance = curr_street_distance / 1000.0;
                        double_string = double_to_string(curr_street_distance, 1);  
                        inst = "Turn " + turn_direction + " onto " + curr_street_name + ". (" + double_string + " km)\n";

                    }
                    else {
                        int int_distance = round_distance(curr_street_distance);
                        inst = "Turn " + turn_direction + " onto " + curr_street_name + ". (" + std::to_string(int_distance) + " m)\n";   
                    }
                }
                instructions.push_back(inst);                
            }


        }

    }

    std::string i2 = "Reaches destination at " + getIntersectionName(path_v.back().to_node) + ".\n";
    instructions.push_back(i2);

    // round time (total_time) into minutes
    int rounded_time = round(total_time / 60.0);
    if (rounded_time == 0)  rounded_time = 1;    

    
    std::string i0;
    if (total_distance > 1000) {
        total_distance = total_distance / 1000.0;
        double_string = double_to_string(total_distance, 1);
        i0 = "Route: " + std::to_string(rounded_time) + " min" + " (" + double_string + " km).\n";
    }

    else {
        int rounded_distance = round_distance(total_distance);
        i0 = "Route: " + std::to_string(rounded_time) + " min" + " (" + std::to_string(rounded_distance) + " m).\n";
    }
    
    instructions.insert(instructions.begin(), i0);
    return instructions;

}


double angleFromLatLon(LatLon start, LatLon end) {

    double x_start = x_from_lon(start.longitude());
    double x_end = x_from_lon(end.longitude());
    double y_start = y_from_lat(start.latitude());
    double y_end = y_from_lat(end.latitude());

    double dx = x_end - x_start;
    double dy = y_end - y_start;

    double ratio = dy/dx;

    const double radianToDegree = 180/M_PI;

    // angle: -90 to 90
    double angle = atan(ratio) * radianToDegree;
    // expand to -180 to 180
    if (dx < 0) {
        if (angle < 0) {
            angle += 180;
        }
        else {
            angle -= 180;
        }
    }

    return angle;


}


// Read two street segments from and to (Presumes that they are intersected)
// Figure out whether turning left or right to turn from "from" to "to"
// Return true if turn left
bool turn_left(StreetSegmentIdx from, StreetSegmentIdx to) {
    StreetSegmentInfo from_street = getStreetSegmentInfo(from);
    StreetSegmentInfo to_street = getStreetSegmentInfo(to);


    LatLon starting, intersecting, ending;
    // LatLon         getIntersectionPosition(IntersectionIdx intersectionIdx);
    if (from_street.from == to_street.from) {
        starting = getIntersectionPosition(from_street.to);
        intersecting = getIntersectionPosition(from_street.from);
        ending = getIntersectionPosition(to_street.to);
    }
    else if (from_street.from == to_street.to) {
        starting = getIntersectionPosition(from_street.to);
        intersecting = getIntersectionPosition(from_street.from);
        ending = getIntersectionPosition(to_street.from);
    }
    else if (from_street.to == to_street.from) {
        starting = getIntersectionPosition(from_street.from);
        intersecting = getIntersectionPosition(from_street.to);
        ending = getIntersectionPosition(to_street.to);
    }
    else if (from_street.to == to_street.to) {
        starting = getIntersectionPosition(from_street.from);
        intersecting = getIntersectionPosition(from_street.to);
        ending = getIntersectionPosition(to_street.from);
    }

    double x1 = x_from_lon(starting.longitude());
    double y1 = y_from_lat(starting.latitude());
    double x2 = x_from_lon(intersecting.longitude());
    double y2 = y_from_lat(intersecting.latitude());
    double x3 = x_from_lon(intersecting.longitude());
    double y3 = y_from_lat(intersecting.latitude());
    double x4 = x_from_lon(ending.longitude());
    double y4 = y_from_lat(ending.latitude());

    double cross_product = (x2 - x1) * (y4 - y3) - (y2 - y1) * (x4 - x3);
    if (cross_product > 0 || cross_product == 0) {
        return true;
    }
    else return false;
}


// Round up the distance to the closest 10
int round_distance(double n) {
    return round(n/10) * 10;

}

std::string double_to_string(double value, int precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}