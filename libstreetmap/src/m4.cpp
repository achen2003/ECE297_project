#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "globals.h"


#include <climits>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <utility>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <ctime>
#include <cstdlib>
#include <random>
#include <vector>
#include <mutex>

double rand_double();


double rand_double() {

    srand((unsigned)time(NULL));
    double x = (double)rand()/((double) RAND_MAX);
    return x;
}

const double TIME_LIMIT = 50000;

#define NO_EDGE -1  // Illegal edge ID -> no edge

/*
//Specifies a delivery order (input to your algorithm).
//To satisfy the order the item-to-be-delivered must have been picked-up
//from the pickUp intersection before visiting the dropOff intersection.
struct DeliveryInf {
    // Constructor
    DeliveryInf(IntersectionIdx pick_up, IntersectionIdx drop_off)
        : pickUp(pick_up), dropOff(drop_off) {}
    //The intersection id where the item-to-be-delivered is picked-up.
    IntersectionIdx pickUp;
    //The intersection id where the item-to-be-delivered is dropped-off.
    IntersectionIdx dropOff;
};
// Specifies one subpath of the courier truck route
struct CourierSubPath {
    // The intersection id where a start depot, pick-up intersection or
    // drop-off intersection is located
    IntersectionIdx start_intersection;
    // The intersection id where this subpath ends. This must be the
    // start_intersection of the next subpath or the intersection of an end depot
    IntersectionIdx end_intersection;
    // Street segment ids of the path between start_intersection and end_intersection
    // They form a connected path (see m3.h)
    std::vector<StreetSegmentIdx> subpath;
};

*/

// The timing variable
std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
int64_t elapsed_time;


// Address for the delivery order
struct PickOrDrop {
    bool pick;
    int delivery_idx;
};

struct PathInfo {
    std::vector<PickOrDrop> delivery_path;
    double travel_time;
    std::vector<CourierSubPath> path_vector;
};
// std::vector<StreetSegmentIdx> findPathBetweenIntersections(const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids, const double turn_penalty);

// All useful intersections to be considered in preloading:
// deliveries[i].pickUp
// deliveries[i].dropOff
// depots[i]
void preloading_traveling_path(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots,
        const float turn_penalty); 




std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>> multidestDijkstra(const IntersectionIdx start_intersection, 
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, 
        const double turn_penalty);

std::pair<std::vector<StreetSegmentIdx>, double> bfsTraceBackWithNoDirectionGenerations(IntersectionIdx end_intersection, IntersectionIdx start_intersection, std::vector<Node> &local_nodes, const double turn_penalty);


PathInfo findSolutionFromStartingDepot(IntersectionIdx start_depot, int first_delivery_idx,       
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, const double turn_penalty);


struct ComparePath {
    bool operator()(const PathInfo& p1, const PathInfo& p2) {
        return p1.travel_time > p2.travel_time;
    }
};
std::priority_queue<PathInfo, std::vector<PathInfo>, ComparePath> paths_queue;

std::pair<PathInfo, PathInfo> swapTwoIntersections(std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, const double turn_penalty); 
std::pair<PathInfo, PathInfo> twoOpt(std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, const double turn_penalty);

std::pair<PathInfo, PathInfo> shifting (std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots,  const double turn_penalty);

PathInfo threeOpt(std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, const double turn_penalty);

PathInfo reconnectingPaths(std::vector<PickOrDrop> &temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, double turn_penalty);

// Perform hill-climbing on the delivery path using the two-opt algorithm
PathInfo hillClimbingTwoOpt(PathInfo curr_path, const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots, const double turn_penalty);

// Perform hill-climbing on the delivery path using the three-opt algorithm
PathInfo hillClimbingThreeOpt(PathInfo curr_path, const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots, const double turn_penalty);

// Multisource Dijkstra
// Used for preloading data
// Only stop after all interesting intersections are hit
// Stores all paths from the start_intersection
// Key is the reaching intersection (all of the interesting intersections)
// Value is the path from the start_intersection to the reaching_intersection
// Should be looped by plugging in all interesting intersections as the start_intersection
// multidestDijkstra(start_intersection, /* deliveries, depots, turn_penalty...*/)[end_intersection] 
// should return the std::pair of the path and the travel time
std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>> multidestDijkstra(const IntersectionIdx start_intersection, 
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, 
        const double turn_penalty){

    std::unordered_map<IntersectionIdx, char> interesting_intersections;
    // Insert all intersections from deliveries
    for (int i = 0; i < deliveries.size(); ++i) {
        char random_value = '0';
        IntersectionIdx pick_up = deliveries[i].pickUp;
        std::pair<IntersectionIdx, char> pick_up_intersection(pick_up, random_value);
        interesting_intersections.insert(pick_up_intersection);

        IntersectionIdx drop_off = deliveries[i].dropOff;
        std::pair<IntersectionIdx, char> drop_off_intersection(drop_off, random_value);
        interesting_intersections.insert(drop_off_intersection);        
    }
    // Insert all intersections from depots
    for (int i = 0; i < depots.size(); ++i) {
        char random_value = '0';
        std::pair<IntersectionIdx, char> depot_intersection(depots[i], random_value);
        interesting_intersections.insert(depot_intersection);        
    }

    /// The function should end when all interesting_intersections are popped, and the unordered_map of interesting_intersection is empty
    // It fills the following unordered_map
    std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>> returned_map;

    // Initialize the node vector every time
    std::vector<Node> local_nodes;
    local_nodes.resize(getNumIntersections());
    for (IntersectionIdx i = 0; i < getNumIntersections(); ++i) {
        local_nodes[i].nodeID = i;
        local_nodes[i].reachingEdge = NO_EDGE;
        local_nodes[i].visited = false;
        local_nodes[i].bestTime = std::numeric_limits<double>::infinity();
        local_nodes[i].edges.resize(getNumIntersectionStreetSegment(i));
        for (int j = 0; j < getNumIntersectionStreetSegment(i); ++j) {
            local_nodes[i].edges[j] = getIntersectionStreetSegment(i, j); 
        }
    }



    // std::list<WaveElem> wavefront;
    std::priority_queue<WaveElem, std::vector<WaveElem>, CompareWaveElem> pq;
    // wavefront.push_back (WaveElem (start_intersection, NO_EDGE, 0));
    pq.push(WaveElem (start_intersection, NO_EDGE, 0));
    local_nodes[start_intersection].bestTime = 0;

    // StreetSegmentIdx previousEdge = NO_EDGE;
    // account for one way
    // account for turning
    while (!pq.empty()) {
        // Get the node with the smallest bestTime in the wavefront

        WaveElem curr = pq.top();
        pq.pop();

        int currID = curr.nodeID;

        if (local_nodes[currID].visited) { // Only visit once
            continue;
        }

        if (curr.travelTime > local_nodes[currID].bestTime) {
            continue;
        }

        // Was this a better path to this node? Update if so
        local_nodes[currID].reachingEdge = curr.edgeID; 
        local_nodes[currID].bestTime = curr.travelTime;
        local_nodes[currID].visited = true;


        // if currID matches any of the intersting intersections
        auto it = interesting_intersections.find(currID);
        if (it != interesting_intersections.end()) {
            std::pair<std::vector<StreetSegmentIdx>, double> temp_path = bfsTraceBackWithNoDirectionGenerations(currID, start_intersection, local_nodes, turn_penalty);
            interesting_intersections.erase(it);
            returned_map.insert(make_pair(currID,temp_path));
        }

        if (interesting_intersections.empty()) {
            break;
        }


        // each outEdge of curr.nodeID
        for (int i = 0; i < (local_nodes[currID].edges).size(); ++i) {
            StreetSegmentIdx outEdgeID = local_nodes[currID].edges[i];

            StreetSegmentInfo outEdge = getStreetSegmentInfo(outEdgeID);
            int toNodeID;

            if (outEdge.oneWay && outEdge.to == currID) {
                continue;      
            }

            if (outEdge.from == local_nodes[currID].nodeID) {
                toNodeID = outEdge.to;
            }
            else {
                toNodeID = outEdge.from;
            }

            // turn_penalty
            double total_time = local_nodes[currID].bestTime + findStreetSegmentTravelTime(outEdgeID);
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
    
    // Jumps out of the loop when the all interesting intersections are taken care of
    return returned_map;
}



// BFS search
// Tracing back to get the shortest path from start_intersection to end_intersection
// Returns the vector of all Street Segments that form this path
// Basically same as bfsTraceBack, the only difference is that it does not call the direction generator
// to save executing time
// It also returns the travel time
std::pair<std::vector<StreetSegmentIdx>, double> bfsTraceBackWithNoDirectionGenerations(IntersectionIdx end_intersection, IntersectionIdx start_intersection, std::vector<Node> &local_nodes, const double turn_penalty) {
    std::list<Path> path;

    // std::list<StreetSegmentIdx> path;
    IntersectionIdx currNodeID = end_intersection;
    StreetSegmentIdx prevEdge = local_nodes[currNodeID].reachingEdge;

    while (prevEdge != NO_EDGE) {
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

        prevEdge = local_nodes[currNodeID].reachingEdge;

        if (prevEdge == NO_EDGE) {
            path.clear(); // no reaching edge, return an empty path
            break;
        }
    }



    // The return vector for passing the autotester:
    std::vector<StreetSegmentIdx> path_vector;
    for (auto it : path) {
        path_vector.push_back(it.passing_edge);        
    }

    double travel_time = computePathTravelTime(path_vector, turn_penalty);
    if (end_intersection != start_intersection && path.size() == 0) {
        travel_time = std::numeric_limits<double>::max();
    }

    std::pair<std::vector<StreetSegmentIdx>, double> path_time_pair(path_vector,travel_time);


    return path_time_pair;
}




// multidestDijkstra(start_intersection, /* deliveries, depots, turn_penalty...*/)[end_intersection] 
// should return the std::pair of the path and the travel time
// Use multidestDijkstra() to preload the data
std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>>> preloaded_paths;

void preloading_traveling_path(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots,
        const float turn_penalty) {
    
    // Any delivery as the starting point (both)
    int size;
    if (deliveries.size() < depots.size())  size = depots.size();
    else    size = deliveries.size();

    if (!preloaded_paths.empty())   preloaded_paths.clear();
    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>>> local_preloaded_paths;

    #pragma omp parallel for
    for (int i = 0; i < size; ++i) {
        if (i < deliveries.size()) {
            IntersectionIdx pick_up_point = deliveries[i].pickUp;

            if (!preloaded_paths.count(pick_up_point)) {
                auto path1 = multidestDijkstra(pick_up_point, deliveries, depots, turn_penalty);
                local_preloaded_paths.insert(std::make_pair(pick_up_point, std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>>()));
                local_preloaded_paths[pick_up_point] = path1;                
            }

            IntersectionIdx drop_off_point = deliveries[i].dropOff;

            if (!preloaded_paths.count(drop_off_point)) {
                auto path2 = multidestDijkstra(drop_off_point, deliveries, depots, turn_penalty);
                local_preloaded_paths.insert(std::make_pair(drop_off_point, std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>>()));
                local_preloaded_paths[drop_off_point] = path2;                  
            }
        }

        if (i < depots.size()) {
            IntersectionIdx depot_point = depots[i];
            if (!preloaded_paths.count(depot_point)) {
                auto path3 = multidestDijkstra(depot_point, deliveries, depots, turn_penalty);
                local_preloaded_paths.insert(std::make_pair(depot_point, std::unordered_map<IntersectionIdx, std::pair<std::vector<StreetSegmentIdx>, double>>()));
                local_preloaded_paths[depot_point] = path3;                    
            }
        }
    }

    preloaded_paths = local_preloaded_paths;
    local_preloaded_paths.clear();
}



// For each delivery, keep track of its status
// - picked_up ---> Whether the delivery man has to consider picking the package up
// - on_hold ---> Whether the delivery man has to consider dropping the package off
typedef struct {
    std::vector<bool> picked_up;
    std::vector<bool> on_hold;
} delivery_info;
delivery_info delivery_packages;



PathInfo best_solution;


// Given the starting depot, find the best solution greedily
// Update the best_delivery_order and final_path_vector
// Picking a random order
// swapTwoIntersections and twoOpt are called after finding a solution
PathInfo findSolutionFromStartingDepot(IntersectionIdx start_depot, int first_delivery_idx,       
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, const double turn_penalty) {

    // std::cout << "Multithread verifier 1: " << start_depot << std::endl;     
    PathInfo new_path;
    new_path.travel_time = std::numeric_limits<double>::max();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    if (lasted_time > 48000) {
        return new_path;
    }
    std::vector<CourierSubPath> temp_path_vector;
    std::vector<PickOrDrop> temp_delivery_order;
    double temp_time;
    double temp_total_time = 0;

    const int total_package_num = deliveries.size();


    int package_delivered = 0;


    delivery_info temp_delivery_packages;
    // The packages that are on delivery man's hand waited to be delivered
    // (Contains the Idx of the package in the vector deliveries)
    temp_delivery_packages.picked_up.resize(total_package_num);

    // The packages that the delivery man has picked up but not yet delivered
    // The delivery man can go to the destination if the corresponding package is picked up
    temp_delivery_packages.on_hold.resize(total_package_num);
    
    for (int i = 0; i < total_package_num; ++i) {
        temp_delivery_packages.picked_up[i] = false;
        temp_delivery_packages.on_hold[i] = false;
    }
    temp_delivery_packages.picked_up[first_delivery_idx] = true;
    temp_delivery_packages.on_hold[first_delivery_idx] = true;
    // The status of the packages
    // The delivery man can go to the destination if the corresponding package is picked up


    // The first subpath (from depot to the first delivery pickup)
    CourierSubPath first_path;
    first_path.start_intersection = start_depot;
    first_path.end_intersection = deliveries[first_delivery_idx].pickUp;

    first_path.subpath = preloaded_paths[first_path.start_intersection][first_path.end_intersection].first;
    temp_time = preloaded_paths[first_path.start_intersection][first_path.end_intersection].second;
    temp_total_time += temp_time;
    //final_path_vector.push_back(first_path);
    temp_path_vector.push_back(first_path);

    // PickOrDrop first_pick_or_drop;
    // first_pick_or_drop.pick = true;
    // first_pick_or_drop.delivery_idx = first_delivery_idx;
    // temp_delivery_order.push_back(first_pick_or_drop);
    
    PickOrDrop first_pick;
    first_pick.delivery_idx = first_delivery_idx;
    first_pick.pick = true;
    temp_delivery_order.push_back(first_pick);
    // Update the current location
    IntersectionIdx curr_location = first_path.end_intersection;

    // Go from any depot to nearest pickup, p

    while (package_delivered < total_package_num) {
        // p = nearest legal pickup or dropoff
        // solution = path to p
        bool delivery_pick_up = true;
        double min_time = std::numeric_limits<double>::max();
        IntersectionIdx next_intersection = -1;
        CourierSubPath temp_path;
        PickOrDrop temp_pick_or_drop;
        int idx = -1;   // index of the package in the deliveries vector

        // Check all packages that were not picked up yet
        // Find the closest one 
        // next_intersection stores the closest IntersectionIdx
        // idx stores the index of this delivery in the delivery vector
        for (int i = 0; i < deliveries.size(); ++i) {
            if (!temp_delivery_packages.picked_up[i]) {
                temp_time = preloaded_paths[curr_location][deliveries[i].pickUp].second;
                if (temp_time < min_time) {
                    min_time = temp_time;
                    next_intersection = deliveries[i].pickUp;
                    idx = i;
                    temp_pick_or_drop.pick = true;
                }
            }
        }

        // Check all packages that are waited to be delivered
        for (int i = 0; i < deliveries.size(); ++i) {
            if (temp_delivery_packages.on_hold[i]) {
                temp_time = preloaded_paths[curr_location][deliveries[i].dropOff].second;
                if (temp_time < min_time) {
                    delivery_pick_up = false;
                    min_time = temp_time;
                    next_intersection = deliveries[i].dropOff;
                    idx = i;
                    temp_pick_or_drop.pick = false;
                }                
            }

        }

        // Make the move
        temp_path.start_intersection = curr_location;   
        temp_path.end_intersection = next_intersection;
        temp_total_time += min_time;

        if (curr_location != next_intersection) {
            temp_path.subpath = preloaded_paths[curr_location][next_intersection].first;
            temp_path_vector.push_back(temp_path);
        }


        curr_location = temp_path.end_intersection;
        // to change the delivery order in twoOpt
        temp_pick_or_drop.delivery_idx = idx;
        temp_delivery_order.push_back(temp_pick_or_drop);

        // Update the package info
        // After picking up a package
        if (delivery_pick_up) {
            temp_delivery_packages.on_hold[idx] = true;
            temp_delivery_packages.picked_up[idx] = true;
        }


        // After dropping off a package
        else if (!delivery_pick_up) {
            temp_delivery_packages.on_hold[idx] = false;
            package_delivered++;
        }
        // loop through all possible places to go
    }

    // All packages are delivered
    // Find the closest depot to end the delivery

    double min_time = std::numeric_limits<double>::max();
    IntersectionIdx end_depot = -1;
    for (int i = 0; i < depots.size(); ++i) {
        temp_time = preloaded_paths[curr_location][depots[i]].second;
        if (temp_time < min_time) {
            min_time = temp_time;
            end_depot = depots[i];
        }
    }

    temp_total_time += min_time;
    CourierSubPath end_path;
    end_path.start_intersection = curr_location;
    end_path.end_intersection = end_depot;

    end_path.subpath = preloaded_paths[curr_location][end_depot].first;

    temp_path_vector.push_back(end_path);

    // new_path is the first attempt finding a better path from multi-start

    // new_path.delivery_path = temp_delivery_order;
    // new_path.path_vector = temp_path_vector;

    new_path = reconnectingPaths(temp_delivery_order, deliveries, depots, turn_penalty);

    // std::vector<StreetSegmentIdx> path_segment_vector; 
    // for (int i = 0; i < temp_path_vector.size(); ++i) {
    //     for (int j = 0; j < temp_path_vector[i].subpath.size(); ++j) {
    //         path_segment_vector.push_back(temp_path_vector[i].subpath[j]);
    //     } 
        
    // }
    // new_path.travel_time = computePathTravelTime(path_segment_vector, turn_penalty);

    // return new_path;

    // Perform two-opt algorithm on this new_path
    // returns the optimized_path
    // compare the optimized_path with the best_solution
    // update the solution if necessary

    // Clear the previous priority queue first
    // paths_queue = std::priority_queue<PathInfo, std::vector<PathInfo>, ComparePath>{};

    // PathInfo optimized_path = twoOpt(new_path.delivery_path, deliveries, depots, turn_penalty);

    PathInfo optimized_path;

    optimized_path = hillClimbingTwoOpt(new_path, deliveries, depots, turn_penalty);
    
    if (optimized_path.travel_time < new_path.travel_time) {
        return optimized_path;
    }
    else {
        return new_path;
    }
    // return new_path;

    // return optimized_path;
}


/*
struct PickOrDrop {
    bool pick;
    int delivery_idx;
};
*/
// Reconstruct the temp_delivery_order
/*
struct PathInfo {
    std::vector<PickOrDrop> delivery_path;
    double travel_time;
    std::vector<CourierSubPath> path_vector;
};
*/

// Given the delivery order vector (must be valid)
// Reconnect all paths it went through
// Produce the delivery path vector
// and the travel time
PathInfo reconnectingPaths(std::vector<PickOrDrop> &temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, double turn_penalty) {
    // Initialization
    double local_min_time = std::numeric_limits<double>::max();
    double trip_time = 0;
    double local_temp_time;
    std::vector<CourierSubPath> temp_path_vector;

    temp_path_vector.resize(temp_delivery_order.size() + 1);

    // Pick up of the first delivery
    IntersectionIdx start_intersection;
    IntersectionIdx end_intersection = deliveries[temp_delivery_order[0].delivery_idx].pickUp;
    for (int k = 0; k < depots.size(); ++k) {
        local_temp_time = preloaded_paths[depots[k]][end_intersection].second;
        if (local_temp_time < local_min_time) {
            local_min_time = local_temp_time;
            start_intersection = depots[k];    
        }
    }
    // From start depot to the first pick-up
    std::vector<StreetSegmentIdx> start_path = preloaded_paths[start_intersection][end_intersection].first;
    trip_time += preloaded_paths[start_intersection][end_intersection].second;


    CourierSubPath start_sub_path;
    start_sub_path.start_intersection = start_intersection;
    start_sub_path.end_intersection = end_intersection;
    start_sub_path.subpath = start_path;

    temp_path_vector[0] = start_sub_path;
    // temp_path_vector.push_back(start_sub_path);

    // Final delivery to the depot
    start_intersection = deliveries[temp_delivery_order[temp_delivery_order.size() - 1].delivery_idx].dropOff;
    local_min_time = std::numeric_limits<double>::max();
    for (int k = 0; k < depots.size(); ++k) {
        local_temp_time = preloaded_paths[start_intersection][depots[k]].second;
        if (local_temp_time < local_min_time) {
            local_min_time = local_temp_time;
            end_intersection = depots[k];    
        }
    }           
    std::vector<StreetSegmentIdx> end_path = preloaded_paths[start_intersection][end_intersection].first; 
    trip_time += preloaded_paths[start_intersection][end_intersection].second;
    CourierSubPath end_sub_path;
    end_sub_path.start_intersection = start_intersection;
    end_sub_path.end_intersection = end_intersection;
    end_sub_path.subpath = end_path;

    
    // Compute the travel time, and compare with best_total_time
    for (int k = 0; k < (temp_delivery_order.size() - 1); ++k) {
        if (temp_delivery_order[k].pick) {
            start_intersection = deliveries[temp_delivery_order[k].delivery_idx].pickUp;
        }
        else if (!temp_delivery_order[k].pick){
            start_intersection = deliveries[temp_delivery_order[k].delivery_idx].dropOff;
        }

        if (temp_delivery_order[k + 1].pick) {
            end_intersection = deliveries[temp_delivery_order[k + 1].delivery_idx].pickUp;
        }
        else if (!temp_delivery_order[k + 1].pick) {
            end_intersection = deliveries[temp_delivery_order[k + 1].delivery_idx].dropOff;
        }
        std::vector<StreetSegmentIdx> mid_path = preloaded_paths[start_intersection][end_intersection].first; 
        trip_time += preloaded_paths[start_intersection][end_intersection].second;
        CourierSubPath mid_sub_path;
        mid_sub_path.start_intersection = start_intersection;
        mid_sub_path.end_intersection = end_intersection;
        mid_sub_path.subpath = mid_path;
        temp_path_vector[k+1] = mid_sub_path;
        // temp_path_vector.push_back(mid_sub_path);
        mid_sub_path.subpath.clear();

    }

    temp_path_vector[temp_path_vector.size() - 1] = end_sub_path;
    // temp_path_vector.push_back(end_sub_path);

    std::vector<StreetSegmentIdx> path_segment_vector;
    for (int i = 0; i < temp_path_vector.size(); ++i) {
        int path_size = temp_path_vector[i].subpath.size();
        for (int j = 0; j < path_size; ++j) {
           path_segment_vector.push_back(temp_path_vector[i].subpath[j]); 
        }
        
    }

    PathInfo res;
    res.travel_time = computePathTravelTime(path_segment_vector, turn_penalty);
    res.delivery_path = temp_delivery_order;
    res.path_vector = temp_path_vector;

    temp_delivery_order.clear();
    temp_path_vector.clear();
    return res;
}



// For swap intersections:
// Takes in a valid delivery order
// Try disconnecting and reconnecting every two single 
// Compute the travel time if the order is valid
// Return the path that has the best travel time
// It also fills the priority_queue paths_queue with the best solution from twoOpt at the top
std::pair<PathInfo, PathInfo> swapTwoIntersections(std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots, const double turn_penalty) {
    
    int size = temp_delivery_order.size();


    std::vector<PickOrDrop> initial_delivery_order = temp_delivery_order;

    std::pair<PathInfo, PathInfo> result;
    result.first.travel_time = std::numeric_limits<double>::max();
    result.second.travel_time = std::numeric_limits<double>::max();

    for (int i = 0; i < size; ++i) {
        for (int j = i; j < size; ++j) {
            if (i == j)     continue;

            std::vector<PickOrDrop> local_delivery_order = initial_delivery_order;
            std::swap(local_delivery_order[i], local_delivery_order[j]);

            // Check if the new path is legal
            std::vector<bool> picked(deliveries.size());
            for (int k = 0; k < picked.size(); ++k) {
                picked[k] = false;
            }
            bool reset = false;
            for (int k = 0; k < local_delivery_order.size(); ++k) {
                // if the order is to drop off sth
                // check if the order is picked up
                // if it's not, the swap is invalid
                if (local_delivery_order[k].pick) {
                    picked[local_delivery_order[k].delivery_idx] = true;
                    continue;
                }
                else /* if (!test_order[k].pick) */{
                    if (!picked[local_delivery_order[k].delivery_idx]) {
                        reset = true;
                        break;
                    }
                }
            }
            if (reset)  {
                local_delivery_order.clear();
                continue;
            }
            picked.clear();

            // If it is legal, compute the new path
            // And find the closest starting and ending depot
            PathInfo new_path = reconnectingPaths(local_delivery_order, deliveries, depots, turn_penalty);

            // Only update the paths_qeueu at the first time
            // if (first_time) {
            //     paths_queue.push(new_path);
            // }
            
            // Update the best_total_time, final_path_vector, and best_delivery_order if better

            if (new_path.travel_time < result.first.travel_time) {
                result.second = result.first;
                result.first= new_path;        
            }      
              
            auto end_time = std::chrono::high_resolution_clock::now();
            auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (lasted_time > TIME_LIMIT * 0.95) {
                return result;
            }   

            // Revert back
            local_delivery_order.clear();

        }
    }

    return result;
}

std::pair<PathInfo, PathInfo> shifting (std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots,  const double turn_penalty) {
    int size = temp_delivery_order.size();


    if (size > 300) {
        size = size / 2;
    }

    std::vector<PickOrDrop> initial_delivery_order = temp_delivery_order;

    std::pair<PathInfo, PathInfo> result;
    result.first.travel_time = std::numeric_limits<double>::max();
    result.second.travel_time = std::numeric_limits<double>::max();

    for (int i = 0; i < size; ++i) {
        for (int j = i; j < size; ++j) {
            if (i >= j)     continue;
            auto end_time = std::chrono::high_resolution_clock::now();
            auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (lasted_time > TIME_LIMIT * 0.95) {
                return result;
            } 
            std::vector<PickOrDrop> local_delivery_order = initial_delivery_order;
            PickOrDrop temp_pick_drop = local_delivery_order[j];
            local_delivery_order[j] = local_delivery_order[i];
            local_delivery_order[i] = temp_pick_drop;


            // Check if the new path is legal
            std::vector<bool> picked(deliveries.size());
            for (int k = 0; k < picked.size(); ++k) {
                picked[k] = false;
            }
            bool reset = false;
            for (int k = 0; k < local_delivery_order.size(); ++k) {
                // if the order is to drop off sth
                // check if the order is picked up
                // if it's not, the swap is invalid
                if (local_delivery_order[k].pick) {
                    picked[local_delivery_order[k].delivery_idx] = true;
                    continue;
                }
                else /* if (!test_order[k].pick) */{
                    if (!picked[local_delivery_order[k].delivery_idx]) {
                        reset = true;
                        break;
                    }
                }
            }
            if (reset)  {
                local_delivery_order.clear();
                continue;
            }
            picked.clear();

            // If it is legal, compute the new path
            // And find the closest starting and ending depot
            PathInfo new_path = reconnectingPaths(local_delivery_order, deliveries, depots, turn_penalty);

            // Only update the paths_qeueu at the first time
            // if (first_time) {
            //     paths_queue.push(new_path);
            // }
            
            // Update the best_total_time, final_path_vector, and best_delivery_order if better
            if (new_path.travel_time < result.first.travel_time) {
                result.second = result.first;
                result.first= new_path;        
            }       
            // end_time = std::chrono::high_resolution_clock::now();
            // lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            // if (lasted_time > TIME_LIMIT) {
            //     return result;
            // }   

            // Revert back
            // local_delivery_order = initial_delivery_order;
            local_delivery_order.clear();

        }
    }

    return result;
}

// For two opt:
// Takes in a valid delivery order
// Try disconnecting and reconnecting every two single 
// Compute the travel time if the order is valid
// Return the path that has the best travel time
// It also fills the priority_queue paths_queue with the best solution from twoOpt at the top
std::pair<PathInfo, PathInfo> twoOpt(std::vector<PickOrDrop> temp_delivery_order, const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots,  const double turn_penalty) {
    
    int size = temp_delivery_order.size();


    if (size > 300) {
        size = size / 2;
    }


    std::vector<PickOrDrop> initial_delivery_order = temp_delivery_order;

    std::pair<PathInfo, PathInfo> result;
    result.first.travel_time = std::numeric_limits<double>::max();
    result.second.travel_time = std::numeric_limits<double>::max();


    for (int i = 0; i < size; ++i) {
        for (int j = i; j < size; ++j) {
            if (i >= j)     continue;
            auto end_time = std::chrono::high_resolution_clock::now();
            auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (lasted_time > TIME_LIMIT * 0.95) {
                return result;
            } 
            std::vector<PickOrDrop> local_delivery_order = initial_delivery_order;
            std::reverse(local_delivery_order.begin() + i, local_delivery_order.begin() + j);

            // Check if the new path is legal
            std::vector<bool> picked(deliveries.size());
            for (int k = 0; k < picked.size(); ++k) {
                picked[k] = false;
            }
            bool reset = false;
            for (int k = 0; k < local_delivery_order.size(); ++k) {
                // if the order is to drop off sth
                // check if the order is picked up
                // if it's not, the swap is invalid
                if (local_delivery_order[k].pick) {
                    picked[local_delivery_order[k].delivery_idx] = true;
                    continue;
                }
                else /* if (!test_order[k].pick) */{
                    if (!picked[local_delivery_order[k].delivery_idx]) {
                        reset = true;
                        break;
                    }
                }
            }
            if (reset)  {
                local_delivery_order = initial_delivery_order;
                continue;
            }
            picked.clear();

            // If it is legal, compute the new path
            // And find the closest starting and ending depot
            PathInfo new_path = reconnectingPaths(local_delivery_order, deliveries, depots, turn_penalty);

            
            // Update the best_total_time, final_path_vector, and best_delivery_order if better
            if (new_path.travel_time < result.first.travel_time) {
                result.second = result.first;
                result.first= new_path;        
            }         


            // Revert back
            // local_delivery_order = initial_delivery_order;
            local_delivery_order.clear();
        }
    }

    return result;
}

// Perform hill-climbing on the delivery path using the two-opt algorithm
// curr_path is the path generated from purely greedy algorithm
// Use that as a start to perform two-opt reconnection
// Perform hillClimbingTwoOpt to each depot (multi-start)
PathInfo hillClimbingTwoOpt(PathInfo curr_path, const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots, const double turn_penalty) {

    PathInfo best_path = curr_path;
    // bool first_time = true;

    // int change_start_path = 5;

    // int counter = 200;
    // std::cout << "Multithread verifier 2: " << /*curr_path.travel_time <<*/ std::endl;  
    double max_temperature;
    double init_cooling_rate;
    double init_delta_cooling_rate;
    int max_iteration;


    std::priority_queue<PathInfo, std::vector<PathInfo>, ComparePath> waitlist_paths;
    // std::vector<PathInfo> waitlist_paths;


    std::unordered_map<double, int> visited_list;

    // int counter = 0;

    if (deliveries.size() < 65) {
        // max_temperature = 100000.0;
        // init_cooling_rate = 0.99999;
        // max_iteration = 5000;
        // init_delta_cooling_rate = 0.001;
        return best_path;
    } 
    else if (deliveries.size() <= 100) {
        if (depots.size() <= 2) {
            max_temperature = 5000.0;
            init_cooling_rate = 0.9;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.01;
        }
        else if (depots.size() <= 5) { // Optimized
            max_temperature = 5000.0;
            init_cooling_rate = 0.9;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.005;            
        }
        else if (depots.size() < 10) {
            max_temperature = 5000.0;
            init_cooling_rate = 0.95;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.005;
        }
        else { // Optimized
            max_temperature = 800.0; ///////
            init_cooling_rate = 0.6;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.01;
        }

    } 
    else if (deliveries.size() <= 150) {
        if (depots.size() <= 2) { 
            max_temperature = 5000.0;
            init_cooling_rate = 0.9;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.01;            
        }
        if (depots.size() <= 5) { 
            max_temperature = 5000.0;
            init_cooling_rate = 0.8;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.005;            
        }
        else {
            max_temperature = 1000.0;
            init_cooling_rate = 0.75;
            max_iteration = 5000; 
            init_delta_cooling_rate = 0.015;              
        }
    }
    else if (deliveries.size() <= 200) {
        if (depots.size() <= 2) {
            max_temperature = 5000.0;
            init_cooling_rate = 0.99;
            max_iteration = 5000;
            init_delta_cooling_rate = 0.005;            
        }
        else if (depots.size() <= 5) {
            max_temperature = 5000.0;
            init_cooling_rate = 0.9;
            max_iteration = 5000;    
            init_delta_cooling_rate = 0.005;
        }
        else { // 
            max_temperature = 5000.0;
            init_cooling_rate = 0.99;
            max_iteration = 10000; 
            init_delta_cooling_rate = 0.005;           
        }

    } 
    else {
        if (depots.size() > 5) {
            max_temperature = 10000.0;
            init_cooling_rate = 0.95;
            max_iteration = 20000;    
            init_delta_cooling_rate = 0.005;
        }
        else {
            max_temperature = 15000.0;
            init_cooling_rate = 0.99;
            max_iteration = 15000;
            init_delta_cooling_rate = 0.001;
        }

    }

    double temperature = max_temperature;
    double min_temperature = 1e-8;
    double cooling_rate = init_cooling_rate;
    int iteration = 0;

    int use_suboptimal_paths = 30;

    // curr_path keep track of the best path so far
    // new_path is the new_explored path
    while ((temperature > min_temperature && iteration < max_iteration) || use_suboptimal_paths > 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        if (lasted_time > TIME_LIMIT * 0.95) {
            break;
        } 

        if (temperature <= min_temperature || iteration >= max_iteration) {
            if (use_suboptimal_paths > 0) {
                use_suboptimal_paths--;
                temperature = max_temperature;
                iteration = 0;
                if (waitlist_paths.size() == 0) break;
                curr_path = waitlist_paths.top();
                waitlist_paths.pop();
                while(true) {
                    if (waitlist_paths.top().travel_time == curr_path.travel_time) {
                        waitlist_paths.pop();
                    }
                    else {
                        break;
                    }
                }
                // curr_path = waitlist_paths[0];
                // waitlist_paths.erase(waitlist_paths.begin());
                // std::cout << "Less optimal path is used as start" << std::endl;
            }
        }
        std::pair<PathInfo, PathInfo> two_opt_paths = twoOpt(curr_path.delivery_path, deliveries, depots, turn_penalty);
        PathInfo best_two_opt_path = two_opt_paths.first;
        
        std::pair<PathInfo, PathInfo> shifted_paths = shifting(curr_path.delivery_path, deliveries, depots, turn_penalty);
        PathInfo best_shifted_path = shifted_paths.first;




        // first_time = false;

        // std::pair<PathInfo, PathInfo> swapped_paths = swapTwoIntersections(curr_path.delivery_path, deliveries, depots, turn_penalty);
        // PathInfo best_swapped_path = swapped_paths.first;

        // Put some second best paths to the waitlist...
        // Try some of the waitlisted paths later...

        waitlist_paths.push(two_opt_paths.second);
        waitlist_paths.push(shifted_paths.second);

        // waitlist_paths.push_back(two_opt_paths.second);
        // waitlist_paths.push_back(shifted_paths.second);
        // waitlist_paths.push(swapped_paths.second);
        

        PathInfo new_path;
        // if (best_two_opt_path.travel_time > best_swapped_path.travel_time) {
        //     new_path = best_swapped_path;
        // }
        // else {
        //     new_path = best_two_opt_path;
        // }
        // if (new_path.travel_time > best_shifted_path.travel_time) {
        //     new_path = best_shifted_path;
        // }

        if (best_shifted_path.travel_time > best_two_opt_path.travel_time && rand_double() <= 0.8) {
            new_path = best_two_opt_path;
        }
        else {
            new_path = best_shifted_path;
        }



        if (visited_list.find(new_path.travel_time) == visited_list.end()) {
            std::pair<double, int> visited_path(new_path.travel_time, 0);
            visited_list.insert(visited_path);
            // std::cout << "New path discovered" << std::endl;
        }
        else {
            // std::cout << "Same path visited" << std::endl;
            visited_list[new_path.travel_time]++;
            if (visited_list[new_path.travel_time] == 2) {
                // if (new_path.travel_time == best_shifted_path.travel_time || new_path.travel_time == best_two_opt_path.travel_time) {
                    if (use_suboptimal_paths > 0) {
                        use_suboptimal_paths--;
                        temperature = max_temperature;
                        iteration = 0;
                        // std::cout << "Using a worse path" << std::endl;
                        // if (waitlist_paths.size() == 0) break;
                        // curr_path = waitlist_paths[0];
                        // waitlist_paths.erase(waitlist_paths.begin());
                        curr_path = waitlist_paths.top();
                        waitlist_paths.pop();
                        while(true) {
                            if (waitlist_paths.top().travel_time == curr_path.travel_time) {
                                waitlist_paths.pop();
                            }
                            else {
                                break;
                            }
                        }
                    }

                // }
                // else if (new_path.travel_time == best_shifted_path.travel_time) {
                //     new_path = best_two_opt_path;
                //     continue;
                // } 
                // else if (new_path.travel_time == best_two_opt_path.travel_time) {
                //     new_path = best_shifted_path;
                //     continue;
                // }
            }

        }

        // new_path = best_shifted_path;

        double energy_diff = new_path.travel_time - curr_path.travel_time;

        if (energy_diff < 0) {
            curr_path = new_path;
        }
        if (new_path.travel_time < best_path.travel_time) {
            best_path = new_path;
            // std::cout << "Better path is found" << std::endl;
            // counter++;
            // std::cout << "Counter: " << counter << std::endl;
        }
        // Not a better path
        else if (energy_diff >= 0) {
            // add some randomness
            // curr_path keeps moving sometimes even if the path is not better

            if (rand_double() < exp(-energy_diff / temperature)) {
                curr_path = new_path;
            }
        }
    
        temperature *= cooling_rate;
        cooling_rate -= init_delta_cooling_rate;
        iteration ++;
    }

    visited_list.clear();
    return best_path;
}





std::vector<CourierSubPath> travelingCourier(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<IntersectionIdx>& depots,
        const float turn_penalty) {

    // return empty path if no delivery at all
    if (deliveries.size() == 0) {
        std::vector<CourierSubPath> empty;
        return empty;
    }

    // Initialization for the path finding
    preloaded_paths.clear();

    best_solution.delivery_path.clear();
    best_solution.path_vector.clear();
    best_solution.travel_time = std::numeric_limits<double>::max();

    // Start the timer
    start_time = std::chrono::high_resolution_clock::now();

    // initialize and re-fill the preloaded data
    // Fill all the paths that are needed 
    std::cout << "Preloading paths... " << std::endl;
    preloading_traveling_path(deliveries, depots, turn_penalty);

    // Calculate the time used in preloading
    auto preload_finished_time = std::chrono::high_resolution_clock::now();
    auto preload_time = std::chrono::duration_cast<std::chrono::milliseconds>(preload_finished_time - start_time).count();
    std::cout << "The time used in preloading data is: " << preload_time << "ms" << std::endl;

    const int total_package_num = deliveries.size();    
    std::cout << "There are " << total_package_num << " packages in total." << std::endl;

    const int total_depot_num = depots.size();
    std::cout << "There are " << total_depot_num << " depots in total." << std::endl;


    // -------------------------------- Start the path finding ---------------------------------------------------------------

    // first is the depot travel time to the nearest pick-up
    // second is the depot index and the nearest pick-up index
    // Sort the depots in the order of the travel_time to the nearest pick_up point
    std::vector<std::pair<double, std::pair<int, int>>> depots_in_order;
    for (int i = 0; i < depots.size(); ++i) {
        for (int j = 0; j < deliveries.size(); ++j) {
            double temp_time = preloaded_paths[depots[i]][deliveries[j].pickUp].second;
            std::pair<int, int> index_pair(i, j);
            std::pair<double, std::pair<int, int>> depot_pair(temp_time, index_pair);
            depots_in_order.push_back(depot_pair);
        }
    }
    sort(depots_in_order.begin(), depots_in_order.end());


    // Decide the number of starting point that we want to use (multi-start)
    int num_depot_choices;
    if (depots_in_order.size() > 4000) {
        num_depot_choices = 4000;
    }
    else num_depot_choices = depots_in_order.size();
    // Start to look into different choices
    // Multi start

    std::vector<PathInfo> parallel_paths(num_depot_choices);
    for (int i = 0; i < parallel_paths.size(); ++i) {
        parallel_paths[i].travel_time = std::numeric_limits<double>::infinity();
    }

    std::mutex mu;
    if (deliveries.size() >= 65) {
        #pragma omp parallel for 
        for (int choice = 0; choice < num_depot_choices; ++choice) {
            
            // std::lock_guard<std::mutex> guard(mu);
            IntersectionIdx start_depot = depots[depots_in_order[choice].second.first];
            int first_delivery_idx = depots_in_order[choice].second.second;

            auto end_time = std::chrono::high_resolution_clock::now();
            auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (lasted_time <= TIME_LIMIT * 0.95) {
                parallel_paths[choice] = findSolutionFromStartingDepot(start_depot, first_delivery_idx, deliveries, depots, turn_penalty);
                // std::cout << "Choice: " << choice << std::endl;
                // std::cout << "A new start depot is used " << std::endl;
            }

        }
    }
    else {
        for (int choice = 0; choice < num_depot_choices; ++choice) {
            IntersectionIdx start_depot = depots[depots_in_order[choice].second.first];
            int first_delivery_idx = depots_in_order[choice].second.second;

            auto end_time = std::chrono::high_resolution_clock::now();
            auto lasted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (lasted_time <= TIME_LIMIT * 0.95) {
                parallel_paths[choice] = findSolutionFromStartingDepot(start_depot, first_delivery_idx, deliveries, depots, turn_penalty);
                // std::cout << "Choice: " << choice << std::endl;
                // std::cout << "A new start depot is used " << std::endl;
            }

        }        
    }


    double best_time = std::numeric_limits<double>::infinity();
    for (int i = 0 ; i < num_depot_choices; ++i) {
        if (parallel_paths[i].travel_time < best_time) {
            best_time = parallel_paths[i].travel_time;
            best_solution = parallel_paths[i];
            std::cout << "Best solution comes from: " << i << std::endl;
        }
    }
    // Local permutations


    paths_queue = std::priority_queue<PathInfo, std::vector<PathInfo>, ComparePath>{};
    auto end_time = std::chrono::high_resolution_clock::now();
    elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Elapsed time: " << elapsed_time << "ms" << std::endl;

    double best_solution_time = 0;
    for (int i = 0; i < best_solution.path_vector.size(); ++i) {
        IntersectionIdx starting_intersection = best_solution.path_vector[i].start_intersection;
        IntersectionIdx ending_intersection = best_solution.path_vector[i].end_intersection;
        best_solution_time += preloaded_paths[starting_intersection][ending_intersection].second;
    }
    std::cout << "Best travel time is " << best_solution_time << std::endl;
    if (best_solution_time == std::numeric_limits<double>::max()) {
        std::vector<CourierSubPath> empty_path;
        return empty_path;
    }
    return best_solution.path_vector;

}

