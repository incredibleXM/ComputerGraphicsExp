#include "ray.h"
#include "material.h"
#include "light.h"

#include "../fileio/imageio.h"

using namespace std;
extern bool debugMode;


// Apply the Phong model to this point on the surface of the object, returning
// the color of that point.
Vec3d Material::shade( Scene *scene, const ray& r, const isect& i, bool isInAir ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the Phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.
	if( debugMode )
		std::cout << "Debugging the Phong code" << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( vector<Light*>::const_iterator litr = scene->beginLights();
	// 		litr != scene->endLights();
	// 		++litr )
	// {
	// 		Light* pLight = *litr;
	// 		.
	// 		.
	// 		.
	// }

	const Vec3d point = r.at(i.t);

	Vec3d shade = ke(i);

	Vec3d ambient = prod(ka(i), scene->ambient());
	ambient = prod(kt(i), ambient);
	if (!isInAir)
		ambient = prod(kt(i), ambient);
    shade += ambient;

	for(vector<Light*>::const_iterator iter = scene->beginLights();
		iter != scene->endLights(); ++iter) {
		const Light* light = *iter;
        const Vec3d lightDirection = light->getDirection(point);
        const Vec3d shadowAttenuation = light->shadowAttenuation(point);
        const double distanceAttenuation = light->distanceAttenuation(point);
        const Vec3d intensity = prod(light->getColor(point), shadowAttenuation * distanceAttenuation);
        if (debugMode) cerr << "Shadow: " << shadowAttenuation << ", Distance: " << distanceAttenuation << endl;

        const Vec3d diffuseTerm = kd(i) * max(0.0, i.N * lightDirection);

        const Vec3d reflectionDir = 2.0 * (lightDirection * i.N) * i.N - lightDirection;
        const Vec3d viewingDir = -r.getDirection();
        const Vec3d specularTerm = ks(i) * pow(max(0.0, viewingDir * reflectionDir), shininess(i));

        shade += prod(intensity, diffuseTerm + specularTerm);
        if (debugMode) cerr << "Diffuse: " << diffuseTerm << ", Specular: " << specularTerm << endl;
	}

	return shade;
}


TextureMap::TextureMap( string filename )
{
    data = load( filename.c_str(), width, height );
    if( 0 == data )
    {
        width = 0;
        height = 0;
        string error( "Unable to load texture map '" );
        error.append( filename );
        error.append( "'." );
        throw TextureMapException( error );
    }
}

Vec3d TextureMap::getMappedValue( const Vec2d& coord ) const
{
	// YOUR CODE HERE

    // In order to add texture mapping support to the
    // raytracer, you need to implement this function.
    // What this function should do is convert from
    // parametric space which is the unit square
    // [0, 1] x [0, 1] in 2-space to Image coordinates,
    // and use these to perform bilinear interpolation
    // of the values.

	const double* vec = coord.getPointer();
	double u = vec[0] * width, v = vec[1] * height;

	int i = floor(u), j = floor(v);
	double du = u - i, dv = v - j;

	Vec3d Tij = getPixelAt(i, j);
	Vec3d Ti1j = getPixelAt(i+1, j);
	Vec3d Tij1 = getPixelAt(i, j+1);
	Vec3d Ti1j1 = getPixelAt(i+1, j+1);

	// bilinear interpolation
	Vec3d Tidjd = (1.0 - du) * (1.0 - dv) * Tij + du * (1.0 - dv) * Ti1j
				+ (1.0 - du) * dv * Tij1 + du * dv * Ti1j1;

    return Tidjd;
}


Vec3d TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return Vec3d(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return Vec3d( double(data[pos]) / 255.0,
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0 );
}

Vec3d MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        Vec3d value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}
