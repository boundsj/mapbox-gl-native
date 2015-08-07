#import "MGLMapCamera.h"

#include <mbgl/util/projection.hpp>

@implementation MGLMapCamera

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)camera
{
    return [[self alloc] init];
}

+ (instancetype)cameraLookingAtCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate
                              fromEyeCoordinate:(CLLocationCoordinate2D)eyeCoordinate
                                    eyeAltitude:(CLLocationDistance)eyeAltitude
{
    mbgl::LatLng centerLatLng = mbgl::LatLng(centerCoordinate.latitude, centerCoordinate.longitude);
    mbgl::LatLng eyeLatLng = mbgl::LatLng(eyeCoordinate.latitude, eyeCoordinate.longitude);
    
    mbgl::ProjectedMeters centerMeters = mbgl::Projection::projectedMetersForLatLng(centerLatLng);
    mbgl::ProjectedMeters eyeMeters = mbgl::Projection::projectedMetersForLatLng(eyeLatLng);
    CLLocationDirection heading = std::tan((centerMeters.northing - eyeMeters.northing) /
                                           (centerMeters.easting - eyeMeters.easting));
    return [[self alloc] initWithCenterCoordinate:centerCoordinate
                                         altitude:eyeAltitude
                                          heading:heading];
}

+ (instancetype)cameraLookingAtCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate
                                   fromDistance:(CLLocationDistance)distance
                                        heading:(CLLocationDirection)heading
{
    return [[self alloc] initWithCenterCoordinate:centerCoordinate
                                         altitude:distance
                                          heading:heading];
}

- (instancetype)initWithCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate
                                altitude:(CLLocationDistance)altitude
                                 heading:(CLLocationDirection)heading
{
    if (self = [super init])
    {
        _centerCoordinate = centerCoordinate;
        _altitude = altitude;
        _heading = heading;
    }
    return self;
}

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    if (self = [super init])
    {
        _centerCoordinate = CLLocationCoordinate2DMake([[coder decodeObjectForKey:@"centerLatitude"] doubleValue],
                                                       [[coder decodeObjectForKey:@"centerLongitude"] doubleValue]);
        _altitude = [[coder decodeObjectForKey:@"altitude"] doubleValue];
        _heading = [[coder decodeObjectForKey:@"heading"] doubleValue];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeDouble:_centerCoordinate.latitude forKey:@"centerLatitude"];
    [coder encodeDouble:_centerCoordinate.longitude forKey:@"centerLongitude"];
    [coder encodeDouble:_altitude forKey:@"altitude"];
    [coder encodeDouble:_heading forKey:@"heading"];
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    return [[[self class] allocWithZone:zone] initWithCenterCoordinate:_centerCoordinate
                                                              altitude:_altitude
                                                               heading:_heading];
}

@end
