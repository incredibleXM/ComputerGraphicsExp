#include <cmath>

#include "light.h"


using namespace std;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.

	const Vec3d direction = getDirection(P);
    ray r(P + direction * RAY_EPSILON, direction, ray::SHADOW);
    isect i;
    Vec3d attenuation = Vec3d(1, 1, 1);
    while (scene->intersect(r, i)) {
		attenuation = prod(attenuation, i.getMaterial().kt(i));
        r = ray(r.at(i.t) + direction * RAY_EPSILON, direction, ray::SHADOW);
    }

    return attenuation;
}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
	// Color doesn't depend on P
	return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity
	// of the light based on the distance between the source and the
	// point P.  For now, we assume no attenuation and just return 1.0

	const double distance = (position - P).length();
	return min(1.0, 1.0 / (constantTerm
                           + linearTerm * distance
                           + quadraticTerm * pow(distance, 2)));
}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
	// Color doesn't depend on P
	return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
	Vec3d ret = position - P;
	ret.normalize();
	return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	const Vec3d direction = getDirection(P);
    ray r(P + direction * RAY_EPSILON, direction, ray::SHADOW);
    isect i;
    Vec3d attenuation(1, 1, 1);
    while (scene->intersect(r, i)) {
        const Vec3d isectPoint = r.at(i.t);
        if ((isectPoint - P).length() < (position - P).length()) {
            attenuation = prod(attenuation, i.getMaterial().kt(i));
            r = ray(isectPoint + direction * RAY_EPSILON, direction, ray::SHADOW);
        } else break;
    }

    return attenuation;
}
