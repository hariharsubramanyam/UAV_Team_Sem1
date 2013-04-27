#include "defines.h"

extern _Q16 num0p998, num0p0001, num0p001, num1p0, num0p5, 
    num1p1, num2p0, num4p0, num255, numPI, num2125, num875,
    num625, num1500, num2000, num1000, num10000, num500,
    num512, num3685, num14p45;

// Constant definitions
// Hack, find a better way to do this
void InitCnsts(void) {
    num0p998 = _Q16ftoi(0.998);
    num0p0001 = _Q16ftoi(0.0001);
    num0p001 = _Q16ftoi(0.001);
    num1p0 = _Q16ftoi(1.0);
    num0p5 = _Q16ftoi(0.5);
    num1p1 = _Q16ftoi(1.1);
    num2p0 = _Q16ftoi(2.0);
    int16_t tmp = 255;
    int16toQ16(&num255,&tmp);
    numPI = _Q16ftoi(PI);
    num2125 = _Q16ftoi(2125.0);
    num875 = _Q16ftoi(875.0);
    num1500 = _Q16ftoi(1500.0);
    num625 = _Q16ftoi(625.0);
    num2000 = _Q16ftoi(2000.0);
    num1000 = _Q16ftoi(1000.0);
    num10000 = _Q16ftoi(10000.0);
    num500 = _Q16ftoi(500.0);
    num512 = _Q16ftoi(512.0);
    num3685 = _Q16ftoi(3685.0);
    num14p45 = _Q16ftoi(14.45);
}

// Absolute value _Q16
void _Q16abs(_Q16 *val) {
    if (*val < 0){
        *val = _Q16neg(*val);
    }
}

// Saturate _Q16
void _Q16sat(_Q16 * val, _Q16 high, _Q16 low) {
    if (*val > high){
        *val = high;
    }
    else if (*val < low){
        *val = low;
    }
}

// Saturate int
void uint8sat(uint8_t * val, uint8_t high, uint8_t low) {
    if (*val > high){
        *val = high;
    }
    else if (*val < low){
        *val = low;
    }
}

void int16sat(int16_t * val, int16_t high, int16_t low) {
    if (*val > high){
        *val = high;
    }
    else if (*val < low){
        *val = low;
    }
}

// Convert from int16_t to _Q16
void int16toQ16(_Q16 *to, int16_t *from){
    *to = 0;
    BYTE * ptr = (BYTE*)to;
    memcpy(ptr+2, from, 2);
}

// Convert from _Q16 to uint8_t, saturated from 1 to 255
void Q16touint8(uint8_t *to, _Q16 *from){
    _Q16sat(from,num255,num1p0);
    BYTE * ptr = (BYTE*)from;
    *to = *(ptr + 2);
}

// Convert from _Q16 to int16_t
void Q16toint16(int16_t *to, _Q16 *from){
    //_Q16sat(from,num255,num1p0); // TODO: do I need to saturate?
    BYTE * ptr = (BYTE*)from;
    memcpy(to, ptr+2, 2);
}

// Square root function for _Q16
_Q16 _Q16sqrt(_Q16 in){
    _Q16 pow = 32768; // Q16 for 0.5
    _Q16 result = _Q16power(in, pow);
    return result;
}

// Signed multiply of two _Q16's
_Q16 mult( _Q16 a, _Q16 b){

    BYTE flag = 0;

    _Q16 acc = 0;
    _Q16 result = 0;

    if( a < 0 ){
        //a -= 1;
        //a = ~a;
        a = -a;
        flag = ~flag;
    }

    if( b < 0 ){
        //b -= 1;
        //b = ~b;
        b = -b;
        flag = ~flag;
    }

    result = _Q16mac( a, b, acc);

    if( flag ){
        result = ~result;
        result += 1;
    }

    return result;

}