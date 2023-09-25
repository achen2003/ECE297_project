#include <iostream>
#include "m1.h"
#include "globals.h"
#include "cmath"
#include "limits"
#include "climits"
#define _USE_MATH_DEFINES

// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    double R = kEarthRadiusInMeters;
    if (point_1 == point_2){
        return 0;
    }

    double avg_lat = (point_1.latitude() + point_2.latitude()) * kDegreeToRadian / 2.0;

    double x1 = R * point_1.longitude() * kDegreeToRadian * cos(avg_lat);         
    double x2 = R * point_2.longitude() * kDegreeToRadian * cos(avg_lat);       
    
    double y1 = point_1.latitude() * kDegreeToRadian * R;
    double y2 = point_2.latitude() * kDegreeToRadian * R;

    double distance = pow((x1 - x2), 2) + pow((y1 - y2), 2);
    distance = sqrt (distance);
    return distance;
}


// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    return street_segment_length[street_segment_id];
}


// Returns the travel time to drive from one end of a street segment 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
// * the speed can be 0
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return street_segment_time[street_segment_id];

}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){ //https://en.wikipedia.org/wiki/Shoelace_formula
    int n = getNumFeaturePoints(feature_id);

    // At least 3 points to enclose an area
    // the start and end point should be the same
    if (n < 4){
        return 0;
    }

    double x1, x2, y1, y2;
    const double R = kEarthRadiusInMeters;
    LatLon curr1;
    LatLon curr2;
    double area = 0;

    curr1 = getFeaturePoint(feature_id, 0);
    curr2 = getFeaturePoint(feature_id, n - 1);

    // if not closed, the area is 0
    if (curr1.latitude() != curr2.latitude() || curr1.longitude() != curr2.longitude()) {
        return 0;
    }
    
    double min = INT_MAX;
    double max = INT_MIN;
    for (int i = 0; i< n -1; ++i){
        curr1 = getFeaturePoint(feature_id, i);
        if ((curr1.latitude() * kDegreeToRadian)>max){
            max = curr1.latitude() * kDegreeToRadian;
        }
        else if ((curr1.latitude() * kDegreeToRadian)<min){
            min = curr1.latitude() * kDegreeToRadian;
        }
    }
    double avg_lat = (max + min) / (2);
    for (int i = 0; i < n - 1; ++i)
    {
        curr1 = getFeaturePoint(feature_id, i);  
        curr2 = getFeaturePoint(feature_id, i+1);

        x1 = R * curr1.longitude() * kDegreeToRadian * cos(avg_lat);         //kDegreeToRadian; 
        x2 = R * curr2.longitude() * kDegreeToRadian * cos(avg_lat);       //kDegreeToRadian;
    
        y1 = curr1.latitude() * kDegreeToRadian * R;
        y2 = curr2.latitude() * kDegreeToRadian * R;

        area += (x1 * y2 - x2 * y1);
    }
    area = fabs(area / 2.0);

    return area;
}

