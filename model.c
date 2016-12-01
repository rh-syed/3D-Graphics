/*
Partners
	NAME: Syed Rahman, cse32011, 212206975
	NAME: Li Yin, yinl1, 211608973
	Date: November 10, 2016.
 * the scene data structure is created/stored/traversed here
 *
 *	John Amanatides, Oct 2016
 */


#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "artInternal.h"

extern Material	GetCurrentMaterial(void);
extern int	IntersectSphere(Ray *, double *, Vector *);
extern int	IntersectPlane(Ray *, double *, Vector *);
extern int	IntersectCube(Ray *, double *, Vector *);
extern int	IntersectCylinder(Ray *, double *, Vector *);
extern Point	InvTransPoint(Point, Affine *);
extern Vector	InvTransVector(Vector, Affine *), TransNormal(Vector, Affine *);
extern Matrix	MultMatrix(Matrix *, Matrix *);
extern void	InitCamera(void), InitLighting(void), FinishLighting(void);

#define SPHERE		1
#define PLANE		2
#define CUBE		3
#define CYLINDER	4
#define PI 3.14159265358979323846

typedef struct StackNode {
	Affine CTM;
	struct StackNode *next;
} StackNode;

typedef struct ListNode {
	int nodeType;
	Affine affine;
	Material material;
	struct ListNode *next;
} ListNode;

static Matrix identity= {       1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0};
static Affine CTM;
static StackNode *CTMstack;
static ListNode *scene;


char *
art_Start(void)
{
	CTM.TM= identity;
	CTM.inverseTM= identity;
	CTMstack= NULL;
	InitCamera();
	InitLighting();
	scene= NULL;

	return NULL;
}


static void
FreeModel(scene)
	ListNode *scene;
{
	ListNode *node;
	while(scene) {
		node= scene;
		scene= scene->next;
		free((void *) node);
		/* note material node is never removed */
	}
}


char *
art_End(void)
{
	while(CTMstack != NULL)
		(void) art_PopTM();
	FreeModel(scene);
	FinishLighting();
	return NULL;
}


char *
art_InitTM(void)
{
	CTM.TM= identity;
	CTM.inverseTM= identity;
	return NULL;
}


char *
art_PushTM(void)
{
        StackNode *sp;

	sp= (StackNode *) malloc(sizeof(StackNode));
	sp->CTM= CTM;
	sp->next= CTMstack;
	CTMstack= sp;
	return NULL;
}


char *
art_PopTM(void)
{
        StackNode *sp;

        if(CTMstack != NULL) {
                CTM= CTMstack->CTM;
                sp= CTMstack;
                CTMstack= CTMstack->next;
                free((void *) sp);
		return NULL;
        }
        else	return "stack underflow";
}


static void
ApplyAffine(Affine trans)
{
	CTM.TM= MultMatrix(&trans.TM, &CTM.TM);
	CTM.inverseTM= MultMatrix(&CTM.inverseTM, &trans.inverseTM);
}


char *
art_Scale(double sx, double sy, double sz)
{
	Affine trans;

	trans.TM = identity;
	trans.inverseTM = identity;
	
	//Scaling matrix
	trans.TM.m[0][0] = sx;
	trans.TM.m[1][1] = sy;
	trans.TM.m[2][2] = sz;
	
	//inverse scaling matrix
	trans.inverseTM.m[0][0] = 1/sx;
	trans.inverseTM.m[1][1] = 1/sy;
	trans.inverseTM.m[2][2] = 1/sz;

	ApplyAffine(trans);
	return NULL;
}


char *
art_Rotate(char axis, double degrees)
{
	/*
	* Rotion along Z:
			x' = x*cos q - y*sin q
			y' = x*sin q + y*cos q 
			z' = z
			
	* Rotation along X:
			y' = y*cos q - z*sin q
			z' = y*sin q + z*cos q
			x' = x
			
	* Rotation along Y:
			z' = z*cos q - x*sin q
			x' = z*sin q + x*cos q
			y' = y
	*/
	Affine trans;
	
	trans.TM = identity;
	trans.inverseTM = identity;
 
	degrees *= 2*PI/360;
	
	if(axis == 'x')
	{
		trans.TM.m[1][1] = cos(degrees);
		trans.TM.m[1][2] = -sin(degrees);
		trans.TM.m[2][1] = sin(degrees);
		trans.TM.m[2][2] = cos(degrees);
		
		trans.inverseTM.m[1][1] = cos(degrees);
		trans.inverseTM.m[1][2] = sin(degrees);
		trans.inverseTM.m[2][1] = -sin(degrees);
		trans.inverseTM.m[2][2] = cos(degrees);
	}
	else if(axis == 'y')
	{
		trans.TM.m[0][0] = cos(degrees);
		trans.TM.m[0][2] = sin(degrees);
		trans.TM.m[2][0] = -sin(degrees);
		trans.TM.m[2][2] = cos(degrees);
		
		
		trans.inverseTM.m[0][0] = cos(degrees);
		trans.inverseTM.m[0][2] = -sin(degrees);
		trans.inverseTM.m[2][0] = sin(degrees);
		trans.inverseTM.m[2][2] = cos(degrees);
	}
	else if(axis == 'z')
	{
		trans.TM.m[0][0] = cos(degrees);
		trans.TM.m[0][1] = -sin(degrees);
		trans.TM.m[1][0] = sin(degrees);
		trans.TM.m[1][1] = cos(degrees);
		
		trans.inverseTM.m[0][0] = cos(degrees);
		trans.inverseTM.m[0][1] = sin(degrees);
		trans.inverseTM.m[1][0] = -sin(degrees);
		trans.inverseTM.m[1][1] = cos(degrees);
	}
 
	ApplyAffine(trans);
	return NULL;
}


char *
art_Translate(double tx, double ty, double tz)
{
	
	Affine trans;

	trans.TM = identity;
	trans.inverseTM = identity;
	
	//translation matrix
	trans.TM.m[0][3] = tx;
	trans.TM.m[1][3] = ty;
	trans.TM.m[2][3] = tz;
	
	//inverse translation matrix
	trans.inverseTM.m[0][3] = -tx;
	trans.inverseTM.m[1][3] = -ty;
	trans.inverseTM.m[2][3] = -tz;
	
	
	ApplyAffine(trans);
	return NULL;
}

char *
art_Shear(char axis1, char axis2, double shear)
{
	//six cases of shear in total for 3D
	Affine trans;
	int a1, a2;
	
	switch (axis1) {
		case 'x':
			a1 = 0;
			break;
		case 'y':
			a1 = 1;
			break;
		case 'z':
			a1 = 2;
			break;
	}
	
	switch (axis2) {
		case 'x':
			a2 = 0;
			break;
		case 'y':
			a2 = 1;
			break;
		case 'z':
			a2 = 2;
			break;
	}

	trans.TM = identity;
	trans.inverseTM = identity;
	
	if (axis1 == axis2)
	{
		//printf("For shear, Axis1 cannot equal Axis2.");
	}
	else
	{
		trans.TM.m[a1][a2] = shear;
		trans.inverseTM.m[a1][a2] = -shear;
	}
	
	ApplyAffine(trans);
	return NULL;
}


static void AddObject(int nodeType)
{
	ListNode *object;

	object= (ListNode *) malloc (sizeof(ListNode));
	object->nodeType= nodeType;
	object->affine= CTM;
	object->material= GetCurrentMaterial();
	object->next= scene;
	scene= object;
}


char *
art_Sphere()
{
	AddObject(SPHERE);
	return NULL;
}


char *
art_Plane()
{
	AddObject(PLANE);
	return NULL;
}


char *
art_Cube()
{
	AddObject(CUBE);
	return NULL;
}

char *
art_Cylinder()
{
	AddObject(CYLINDER);
	return NULL;
}


/*
 * This function, when passed a ray and list of objects
 * returns a pointer to the closest intersected object in the list
 * (whose t-value is less than t) and updates t and normal to
 * the value of the closest object's normal and t-value.
 * It returns NULL if it find no object closer than t from
 * the ray origin.  If anyHit is true then it returns the
 * first object that is closer than t.
 */
static ListNode *
ReallyIntersectScene(Ray *ray, ListNode *obj, int anyHit, double *t, Vector *normal)
{
	ListNode *closestObj, *resultObj;
	Ray transRay;
	int i, result;

	closestObj= NULL;

	while(obj != NULL) {
		/* transform ray */
		transRay.origin= InvTransPoint(ray->origin, &obj->affine);
		transRay.direction= InvTransVector(ray->direction, &obj->affine);

		/* intersect object */
		switch(obj->nodeType) {

		case SPHERE:
			result= IntersectSphere(&transRay, t, normal);
			break;
		case PLANE:
			result= IntersectPlane(&transRay, t, normal);
			break;
		case CUBE:
			result= IntersectCube(&transRay, t, normal);
			break;
		case CYLINDER:
			result= IntersectCylinder(&transRay, t, normal);
			break;
		}

		/* keep closest intersection */
		if (result == HIT) {
			closestObj= obj;
			if(anyHit)
				return obj;
		}

		obj= obj->next;
	}

	return closestObj;
}


int
IntersectScene(Ray *ray, double *t, Vector *normal, Material *material)
{
	ListNode *closestObj;
	double closestT;
	Vector closestNormal;

	closestT= UNIVERSE;
	closestObj= ReallyIntersectScene(ray, scene, 0, &closestT, &closestNormal);
	if(closestObj != NULL) {
		*t= closestT;
		*normal= TransNormal(closestNormal, &closestObj->affine);
		*material= closestObj->material;
		return HIT;
	}
	else	return MISS;
}


int
ShadowProbe(Ray *ray, double distanceToLight)
{
	ListNode *closestObj;
	double closestT;
	Vector closestNormal;

	closestT= distanceToLight;
	closestObj= ReallyIntersectScene(ray, scene, 1, &closestT, &closestNormal);
	if(closestObj != NULL)
		return HIT;
	else	return MISS;
}
