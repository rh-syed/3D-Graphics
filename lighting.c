/*
	Partners
	NAME: Syed Rahman, cse32011, 212206975
	NAME: Li Yin, yinl1, 211608973
	Date: November 10, 2016.
 * deals with lights/shading functions
 *
 *	John Amanatides, Oct 2016
 */


#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "artInternal.h"

#define CHECKERBOARD    1
#define ZONE_PLATE      2


#define MAX_RECURSION	10

extern double	Normalize(Vector *);
extern Vector	ReflectRay(Vector, Vector);
extern Vector	RefractRay(Vector, Vector, double);
extern int	IntersectScene(Ray *, double *, Vector *, Material *);
extern int	ShadowProbe(Ray *, double);
extern int TransmitRay(Vector, Vector, double, double, Vector *);

typedef struct LightNode {
	Point position;
	double intensity;
	double radius;
	struct LightNode *next;
} LightNode;

static LightNode *lights;
static Color	background;
static Material	currentMaterial;
static Color black= {0.0, 0.0, 0.0}, white= {1.0, 1.0, 1.0};


char *
art_Light(double x, double y, double z, double intensity, double radius)
{
	LightNode *newLight;

	if(intensity <= 0.0 || radius < 0.0)
		return "art_Light: domain error";
	newLight= (LightNode *) malloc(sizeof(LightNode));
	newLight->position.v[0]= x;
	newLight->position.v[1]= y;
	newLight->position.v[2]= z;
	newLight->intensity= intensity;
	newLight->radius= radius;
	newLight->next= lights;
	lights= newLight;

	return NULL;
}


char *
art_Material(Material material)
{
	currentMaterial= material; /* should really check for mistakes */
	return NULL;
}


Material
GetCurrentMaterial(void)
{
	return currentMaterial;
}


char *
art_Background(Color color)
{
	if(color.v[0] < 0.0 || color.v[0] > 1.0 || color.v[1] < 0.0 || color.v[1] > 1.0 || color.v[2] < 0.0 || color.v[2] > 1.0)
		return "art_Background: domain error";
	background= color;
	return NULL;
}

/* for A4 */
static Color
Texture(Material *material, Point position)
{               
	int funnySum;
	double EPSILON= 0.0001;
	double factor;
	Color result;

	switch(material->texture) {

	case CHECKERBOARD: 
		funnySum= floor(position.v[0]+EPSILON)
			+ floor(position.v[1]+EPSILON)
			+ floor(position.v[2]+EPSILON);
		if(funnySum % 2)
			return white;
		else    return material->col;
	case ZONE_PLATE:
		factor= 0.5*cos(DOT(position, position))+0.5;
		TIMES(result, material->col, factor);
		return result;  
	default:                
	return material->col;
	}       
}       

/*
 * a simple shader
 */
static Color
ComputeRadiance(Ray *ray, double t, Vector normal, Material material)
{

  Color myColor, surface, d, s, refl, refr;
  Vector light, hit, r, rf;
  double NdotL, NdotI, LdotR, flux, factor, distance;
  Ray shadow, reflect, refract;
  LightNode *lightPoint;
  
  //determine hitpoint
  (void) Normalize(&normal);
  TIMES(hit, ray->direction, t);
  PLUS(hit, ray-> origin, hit);
  
  //check surface material
  if(material.texture!=0)
	  surface = Texture(&material, hit);
  else surface = material.col;
  
  TIMES(myColor, surface, material.Ka);
  r=ReflectRay(ray->direction, normal);
  rf=RefractRay(ray->direction, normal, (double)material.index);

	reflect.origin=hit;
	reflect.direction=r;
	reflect.generation=ray->generation +1;
	refract.origin=hit;
	refract.direction=rf;
	refract.generation=ray->generation +1;

	if(material.Kt > 0.0 && material.Kr == 0.0 && ray->generation < MAX_RECURSION) {
			if(IntersectScene(&refract, &t, &normal, &material) == HIT){
				refr = ComputeRadiance(&refract, t, normal, material);
				PLUS(myColor, refr, myColor);
			}
	}
	else if(material.Kr > 0.0 && material.Kt == 0.0 && ray->generation < MAX_RECURSION){
		if(IntersectScene(&reflect, &t, &normal, &material) == HIT){
			refl = ComputeRadiance(&reflect, t, normal, material);
			PLUS(myColor, refl, myColor);
		}
	}
	else{
		//perform lighting calculation
  		for(lightPoint = lights; lightPoint != NULL;lightPoint=lightPoint->next){
	  MINUS(light, lightPoint->position, hit);
	  distance = Normalize(&light);
	  
	  //shadow generation
	  shadow.origin=hit;
	  shadow.direction=light;
	  shadow.generation=0;
	  NdotL=DOT(light, normal);
	  
	  if(ShadowProbe(&shadow, distance)!= HIT && NdotL > 0.0) {
		  flux=lightPoint->intensity/(distance*distance);
		  
		  //determine effective contribution factor towards diffuse lighting
		  factor= flux*material.Kd*NdotL;
		  TIMES(d, surface, factor);
		  PLUS(myColor, d, myColor);
		  
		  LdotR= DOT(light, r);
		  if(LdotR > 0.0) {
			  
			  //determine effective contribution factor towards specular lighting
			  factor= flux*material.Ks*pow(LdotR, (double) material.n);
			  TIMES(s,white,factor);
			  PLUS(myColor, s, myColor);
			  }
		}
		
	}
	}
 
	return myColor;
}


Color
GetRadiance(Ray *ray)
{
	double t;
	Vector normal;
	Material material;

	if(IntersectScene(ray, &t, &normal, &material) == HIT)
		return ComputeRadiance(ray, t, normal, material);
	else	return background;
}


void InitLighting()
{
	Material material;

	material.col= white;
	material.Ka= 0.2;
	material.Kd= 0.6;
	material.Ks= 0.7;
	material.n= 50.0;
	material.Kr= 0.0;
	material.Kt= 0.0;
	material.index= 1.0;
	(void) art_Material(material);
	(void) art_Background(black);

	lights= NULL;
}


void FinishLighting()
{
	LightNode *node;

	while(lights) {
		node= lights;
		lights= lights->next;

		free((void *) node);
	}
}
