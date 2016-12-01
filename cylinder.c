/*
	Partners
	NAME: Syed Rahman, cse32011, 212206975
	NAME: Li Yin, yinl1, 211608973
	Date: November 10, 2016.
 * implement ray/cylinder intersection routine
 *	
 
 *	John Amanatides, Oct 2016
 */


#include <math.h>
#include "artInternal.h"

#define EPSILON	(1e-5)

/*
 * compute intersection between ray and a cylinder (-1<= y <= 1, x^2 + z^2 <= 1)
 * Returns MISS if no intersection; otherwise, it returns HIT and
 * sets t to the the distance  to the intersection point and sets the
 * normal vector to the outward facing normal (unit length) at the
 * intersection point.  Note: no intersection is returned if myT >= t
 * (ie my intersection point is further than something else already hit).
 */

int IntersectCylinder(Ray *ray, double *t, Vector *normal)
{
	/*
	* The formula for circle is x^2 + y^2 = 1. The formula for ray is P(t) = E + tD.
	*/

  double a, b, c;
  double t0, t1, y0, y1;
  Point hit;
  
  Point s=ray->origin;
  Vector d=ray->direction;
  
  /*
  * After subtiuting the ray formula into circle formula and expanding it we get the 
  * the quadratic equation a.t2 + b.t + c = 0
  * where a,b and c are the following.
  */
  a= d.v[0]*d.v[0] + d.v[2]*d.v[2];
  b= 2*s.v[0]*d.v[0] + 2*s.v[2]*d.v[2];
  c= s.v[0]*s.v[0] + s.v[2]*s.v[2] - 1.0;
  
  double x = b*b - 4*a*c;
  if(x < 0)
    return MISS;

  //the roots of the quadratic equation
  t0= (-b + sqrt(x))/(2 * a);
  t1= (-b - sqrt(x))/(2 * a);
  
  //organizing roots by ascending order
  if(t0 > t1) {
    double temp= t0;
    t0= t1;
    t1= temp;
  }
  
  //the intersection points
  y0= s.v[1] + t0*d.v[1];
  y1= s.v[1] + t1*d.v[1];

  /*
  *******INTERSECTION LOGIC**************
	  We're raytracing against a capped cylinder with caps at y = 1, and y = -1, so there are now five different 
	  possibilities for intersections.
		- if y0 >1, and y1 >1, then the ray misses the cylinder entirely.
		- if y0 >1, and y1 <1, then the ray hits the cylinder cap placed at +1. if y0 <1 and y0 >-1, then the ray intersects the side of the cylinder.
		- if y0 <-1 and y1 >-1, then the ray hits the cylinder cap placed at -1
		- if y0 <-1 and y1 <-1 then the ray misses the cylinder entirely.
   **************************************
  */
  if (y0<-1)
  {
    if(y1<-1)
      return MISS;
    else
    {
      //hit lower cap
      double th= t0 + (t1-t0) * (y0+1) / (y0-y1);
      if (th<=0) 
        return MISS;
      TIMES(hit, d, th);
      PLUS(hit, s, hit);
      *t= th;
      *normal= hit;
      
      return HIT;
    }
  }
  else if(y0>-1&&y0<=1)
  {
    //hit cylinder
    if(t0<=0)
      return MISS;
  
    TIMES(hit, d, t0);
    PLUS(hit, s, hit);
	hit.v[1] = 0;
    *t= t1;
    *normal= hit;
	
    return HIT;
  }
  else if(y0>1)
  {
    if(y1>1)
      return MISS;
    else
    {
      //hit upper cap
      double th= t0 + (t1-t0) * (y0+1) / (y0-y1);
      if (th<=0) 
        return MISS;
	
      TIMES(hit, d, th);
      PLUS(hit, s, hit);
      *t= th;
      *normal= hit;
      
      return HIT;
    }
  }
  
	//return MISS;
}
