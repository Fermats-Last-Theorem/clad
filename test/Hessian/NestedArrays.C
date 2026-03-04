// RUN: %cladclang %s -I%S/../../include -o NestedArrays.out 2>&1 | %filecheck %s
// RUN: ./NestedArrays.out | %filecheck_exec %s

#include "clad/Differentiator/Differentiator.h"
#include <stdio.h>
#include <math.h>

double inner_5(double* params) {
    return params[0] * params[1] * params[2] 
         + params[3] * params[3] 
         + params[4] * params[0];
}

double outer_5(double* params) {
    return inner_5(params) + sin(params[2]) - exp(params[4]);
}

double inner_local(double* p) { 
    return p[0] * p[1]; 
}

double outer_local(double x, double y) {
    double arr[2] = {x, y}; 
    return inner_local(arr);
}

double inner_const(double* p, double x) { 
    return p[0] * x; 
}

double outer_const(double x) {
    double arr[2] = {2.0, 3.0}; 
    return inner_const(arr, x);
}

// CHECK: clad::ValueAndPushforward<double, double> _t0 = inner_5_pushforward(params, (double[5]){
// CHECK: void inner_5_pushforward_pullback(double *params, double *_d_params, clad::ValueAndPushforward<double, double> _d_y, double *_d_params0, double *_d_d_params)

int main() {
    double p5[5] = {1.5, 2.0, -0.5, 3.0, 1.0};
    
    auto h5 = clad::hessian(outer_5, "params[0:4]");
    auto h_local = clad::hessian(outer_local);
    auto h_const = clad::hessian(outer_const);
    
    double cladH[25] = {0};
    double cladH_local[4] = {0};
    double cladH_const[1] = {0};
    h5.execute(p5, cladH);
    h_local.execute(2.0, 3.0, cladH_local);
    h_const.execute(5.0, cladH_const);

    printf("H[0,0] = %.2f\n", cladH[0]);   
    // CHECK-EXEC: H[0,0] = 0.00
    
    printf("H[0,1] = %.2f\n", cladH[1]);   
    // CHECK-EXEC: H[0,1] = -0.50
    
    printf("H[2,2] = %.4f\n", cladH[12]);  
    // CHECK-EXEC: H[2,2] = 0.4794
    
    printf("H[4,4] = %.4f\n", cladH[24]);  
    // CHECK-EXEC: H[4,4] = -2.7183

    printf("H_local[0,1] = %.2f\n", cladH_local[1]);
    // CHECK-EXEC: H_local[0,1] = 1.00

    printf("H_const[0,0] = %.2f\n", cladH_const[0]);
    // CHECK-EXEC: H_const[0,0] = 0.00

    return 0;
}