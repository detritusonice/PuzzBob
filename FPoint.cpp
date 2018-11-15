// FPoint.cpp: implementation of the FPoint class.
//
//////////////////////////////////////////////////////////////////////


#include "FPoint.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//-----------------------

 FPoint::FPoint( float px,float py)
{
	x=px;
	y=py;
}


//-----------------------
/*
 FPoint::FPoint( const FPoint& dpt)
{
	x=dpt.x;
	y=dpt.y;
}
*/
//-----------------------

FPoint::~FPoint()
{
}

/////////////////////////// IMPLEMENTATION ////////////////////////////

 bool FPoint::operator==(const FPoint& pt) const
{
	return ( x==pt.x && y==pt.y );
}

 bool FPoint::operator!=(const FPoint& pt) const
{
	return ( x!=pt.x || y!=pt.y );
}

//-------------------------------------------
/*
 FPoint& FPoint::operator=(const FPoint &pt)
{
	x=pt.x;
	y=pt.y;

	return *this;
}
*/
//-----------------------

 FPoint FPoint::operator +(const FPoint& pt) const	//point addition
{
	return FPoint(x+pt.x,y+pt.y);
}

//-----------------------

 FPoint& FPoint::operator +=(const FPoint& pt)			//increase by
{
	x+=pt.x;
	y+=pt.y;
	return *this;
}

//-----------------------

 float FPoint::operator *(const FPoint& pt) const  //dot product
{
	return ( x*pt.x + y*pt.y );
}

//-----------------------

 FPoint FPoint::operator -(const FPoint& pt) const			//point subtraction
{
	return ( FPoint( x-pt.x, y-pt.y ) );
}

//-----------------------

 FPoint& FPoint::operator -=(const FPoint& pt)			//subtract offset point
{
	x-=pt.x;
	y-=pt.y;
	return *this;
}

//-----------------------

 FPoint FPoint::operator -() //negation
{
	return ( FPoint(-x,-y) );
}

//-----------------------

 FPoint FPoint::operator /(float scaling) const	  //used in scaling-normalization
{
	if (scaling != 0.)
		return( FPoint(x/scaling,y/scaling) );
	else
	{
		return (FPoint(x,y));//should throw???
	}
}

//-----------------------

 FPoint& FPoint::operator /=(float scaling)	  //used in normalization in-comp
{
	if (scaling != 0.)
	{
		x/=scaling;
		y/=scaling;
	}

	return *this;
}

//-----------------------

 FPoint FPoint::operator *(float scaling) const  //used in scaling-normalization
{
	return( FPoint(x*scaling,y*scaling) );
}

//-----------------------

 FPoint& FPoint::operator *=(float scaling)	  //used in normalization in-comp
{
	x*=scaling;
	y*=scaling;

	return *this;
}

//-----------------------

 double FPoint::GetLength() const	//distance from origin or length of vector
{
	return ( sqrt (double(x)*x +double(y)*y) );
}

//-----------------------

 FPoint FPoint::GetNormal() const//Get the unit vector
{
	float len=GetLength();
	if (len==0.)
	{
		return (FPoint(1.,0.));
	}
	else
		return( FPoint( x/len, y/len ));
}

//------------------------------
FPoint& FPoint::ProjectOn(const FPoint& projector) //project this vector on another
{
	FPoint proj_normal=projector.GetNormal();

	float proj_length= (*this)*proj_normal;//dot product
	//might be negative, means that resulting direction is reversed

	(*this)=proj_normal;
	(*this)*=proj_length;

	return *this;
}

//------------------------------

 FPoint FPoint::GetProjectionOn(const FPoint& projector)  const
{
	FPoint projection( *this );

	projection.ProjectOn(projector);

	return (projection);
}

//---------------------------------

 FPoint& FPoint::Rotate(const float& theta)//in rads
 {
	 static float px;

	 px=x;

	 x= cos(theta)*px + sin(theta)*y ;
	 y= -sin(theta)*px + cos(theta)*y ;

	 return *this;
 }

FPoint& FPoint::CartesianToPolar()//(x,y) ->(r,8)
{
	float length=this->GetLength();
	if (length==0.) return *this; //(0,0)

	if (x==0.)
	{
		x=length;
		if (y>0)
			y=PI/2.;
		else
			y=-PI/2.;
	}
	else
	{
		y=atan2(y,x);
		x=length;
	}
	return *this;
}
