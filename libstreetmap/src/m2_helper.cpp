#include "m2_helper.h"
#include "m1.h"
#include <cmath>
#include <utility>
#include "globals.h"


float x_from_lon(float lon) {
    return (lon * kDegreeToRadian * 
                  kEarthRadiusInMeters * std::cos(avg_lat_global));
}

float y_from_lat(float lat) {
    return (lat * kDegreeToRadian * kEarthRadiusInMeters);
}

// x = lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat)
// lon = x / (kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat))
float lon_from_x(float x) {
    return (x / (kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat_global)));
}

// y = lat * kDegreeToRadian * kEarthRadiusInMeters
// lat = y / (kDegreeToRadian * kEarthRadiusInMeters)
float lat_from_y(float y) {
    return (y / (kDegreeToRadian * kEarthRadiusInMeters));
}
