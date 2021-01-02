// frac.h

#ifndef FRAC_H
#define FRAC_H

typedef struct RationalNumber {
    int num;
    int denom;
} frac, *frac_p;

float           mult(float num1, float num2);
float           divide(float num1, float num2);
float           add(float num1, float num2);
float           add_id();

#endif //FRAC_H
