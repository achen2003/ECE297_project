/*

Functions:
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id);
IntersectionIdx findClosestIntersection(LatLon my_position);
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id);
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id);
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2);


*/

#include <iostream>
#include "m1.h"
#include "globals.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "m1_helpers.h"


// Returns all intersections reachable by traveling down one street segment 
// from the given intersection (hint: you can't travel the wrong way on a 
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Corner case: cul-de-sacs can connect an intersection to itself 
// (from and to intersection on  street segment are the same). In that case
// include the intersection in the returned vector (no special handling needed).
// Speed Requirement --> high 
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id) {

    std::vector<IntersectionIdx> adjacentIntersections;
    int numOfIntersectionStreetSegments = getNumIntersectionStreetSegment(intersection_id);

    //walk through all intersected street segments from that intersection
    for (int segmentNum = 0; segmentNum < numOfIntersectionStreetSegments; ++segmentNum) {
        StreetSegmentIdx segmentIdx = getIntersectionStreetSegment (intersection_id, segmentNum);
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segmentIdx);

        //don't have to worry about one-way roads if "from"
        if (segmentInfo.from == intersection_id) {

            IntersectionIdx destination = segmentInfo.to;
            adjacentIntersections.push_back(destination);
            continue;                

        }

        else if (segmentInfo.to == intersection_id) {
            if (!segmentInfo.oneWay) {
                IntersectionIdx destination = segmentInfo.from;
                adjacentIntersections.push_back(destination);
                continue;
            }

            else {
                continue;
            }
        }

    }

    //get rid of all duplicated intersections (cul-de-sacs)
    sort(adjacentIntersections.begin(), adjacentIntersections.end() );
    adjacentIntersections.erase( unique( adjacentIntersections.begin(), adjacentIntersections.end() ), adjacentIntersections.end() );

    return adjacentIntersections;
}




// Returns the geographically nearest intersection (i.e. as the crow flies) to 
// the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position) {

    double closestDistance = -1;
    IntersectionIdx closestIdx = -1;
    // initialize the "closest" variables

    for (IntersectionIdx intersection = 0; intersection < getNumIntersections(); ++intersection ) {
  
        LatLon tempLocation = (getIntersectionPosition(intersection));
        double distance = findDistanceBetweenTwoPoints(tempLocation, my_position);
        if (closestDistance == -1) {
            closestDistance = distance;
        }

        else {
            if (closestDistance > distance) {
                closestDistance = distance;
                closestIdx = intersection;
            }
        }
        
    }

    return closestIdx;
}


// this function was copied from the prof's example
// O(n), which is good enough
// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    return intersection_street_segments[intersection_id];
}


// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) {
    return intersections_of_streets[street_id];
}


// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2) {
    std::vector<IntersectionIdx> intersection_street1 = findIntersectionsOfStreet(street_id1);
    std::vector<IntersectionIdx> intersection_street2 = findIntersectionsOfStreet(street_id2);

    std::vector<IntersectionIdx> intersections_of_two_streets;

    sort(intersection_street1.begin(), intersection_street1.end());
    sort(intersection_street2.begin(), intersection_street2.end());


    


    // put the two vectors of intersection into one vector
    set_intersection(
        intersection_street1.begin(), intersection_street1.end(),
        intersection_street2.begin(), intersection_street2.end(),
        std::back_inserter(intersections_of_two_streets));


    return intersections_of_two_streets;



}

