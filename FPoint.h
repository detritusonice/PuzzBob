// FPoint.h: interface for the FPoint class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FPOINT_H
#define FPOINT_H

#ifndef PI
#define PI 3.14159265358979323846f
#endif

class FPoint
{

	public:

	float x,y; //real-world coordinates

	//constructors
	FPoint( float px=0.0, float py=0.0 );//construct from real coords
//	FPoint(const FPoint&);//construct as clone of an existing FPoint //copy constructor not really needed

	virtual ~FPoint();


//	FPoint& operator=(const FPoint); //assignment operator  //not needed

	bool operator==(const FPoint&) const; //equality //absolute
	bool operator!=(const FPoint& ) const;

	FPoint operator +(const FPoint&) const;			//point addition
	FPoint& operator +=(const FPoint&);			//increase by

	FPoint operator -(); //negation
	FPoint operator -(const FPoint&) const;			//point subtraction
	FPoint& operator -=(const FPoint&);			//subtract offset point

	float operator *(const FPoint&) const;  //dot product

	FPoint operator *(float scaling) const;  //used in scaling-normalization
	FPoint& operator *=(float scaling);	  //used in normalization in-comp

	FPoint operator /(float scaling) const;			  //used in scaling-normalization
	FPoint& operator /=(float scaling);	  //used in normalization in-comp

	FPoint& ProjectOn(const FPoint&); //project this vector on another

	double GetLength() const;								//distance from origin or length of vector
	FPoint GetNormal() const;								//Get the unit vector
	FPoint GetProjectionOn(const FPoint&) const;    //Get the Projection vectorof this vector on another

	FPoint& Rotate(const float& theta);
	FPoint& CartesianToPolar();

};
#endif // !defined(AFX_DPOINT_H__5D3A4003_CFE4_11D6_929B_00C0DF233931__INCLUDED_)
