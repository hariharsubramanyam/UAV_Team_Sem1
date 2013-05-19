#include "AHRS.h"

tQuaternion qprod(tQuaternion a, tQuaternion b)
{	
    tQuaternion result;
    result.o = mult(a.o,b.o) - mult(a.x,b.x) - mult(a.y,b.y) - mult(a.z,b.z);
    result.x = mult(a.o,b.x) + mult(a.x,b.o) + mult(a.y,b.z) - mult(a.z,b.y);
    result.y = mult(a.o,b.y) - mult(a.x,b.z) + mult(a.y,b.o) + mult(a.z,b.x);
    result.z = mult(a.o,b.z) + mult(a.x,b.y) - mult(a.y,b.x) + mult(a.z,b.o);
    return result;
}

tQuaternion qprodconj(tQuaternion a, tQuaternion b){
    tQuaternion result;
    a.x = -a.x;
    a.y = -a.y;
    a.z = -a.z;
    result = qprod(a,b);
    return result;
}