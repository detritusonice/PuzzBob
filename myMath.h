#ifndef MY_MATH_H
#define MY_MATH_H

#ifndef PI
#define PI 3.14159265358979323846f
#endif

float mySine(float theta);
float myCosine(float theta);
//void myGenSineTable();

inline float myfmin(float x, float y) {return (x<y)?x:y;}
inline float myfmax(float x, float y) {return (x>y)?x:y;}
inline float myfabs(float x) { return (x>=0)?x:-x;}

#endif // MY_MATH_H
