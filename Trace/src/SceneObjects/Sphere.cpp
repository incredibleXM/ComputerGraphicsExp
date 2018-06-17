#include <cmath>

#include "Sphere.h"

using namespace std;


bool Sphere::intersectLocal( const ray& r, isect& i ) const
{
    // YOUR CODE HERE:
    // Add sphere intersection code here.
    // it currently ignores all spheres and just return false.
    Vec3d p = r.getPosition();
    Vec3d d = r.getDirection();

    double b = -p * d;
    double delta = b*b - p*p + 1;

    if(delta < 0.0) return false;

    delta = sqrt(delta);
    double t2 = b + delta;

    if(t2 <= RAY_EPSILON) return false;

    i.obj = this;
    double t1 = b - delta;

    if(t1 > RAY_EPSILON) {
        i.t = t1;
		i.N = r.at( t1 );
		i.N.normalize();
    } else {
        i.t = t2;
		i.N = r.at( t2 );
		i.N.normalize();
    }

    return true;
}
