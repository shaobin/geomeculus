/*
 * This file includes code reused or adapted from LightAIMD by Bin Shao et al.
 * https://github.com/microsoft/LightAIMD
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */
#include <math.h>
#include <float.h>
#include <complex.h>

#include "numeric_types.h"
#include "constants.h"
#include "special_func.h"

#ifdef __NVCC__
#include <cuComplex.h>
#define C64 cuDoubleComplex

__device__ C64 cuCexp(C64 z)
{
    C64 result;
    sincos(z.y, &result.y, &result.x);
    double d = exp(z.x);
    result.x *= d;
    result.y *= d;
    return result;
}

#else
#define C64 c64
#endif

static u64 const factorial_table[] = {
    1, // 0!
    1, 2, 6, 24, 120, // 1! ~ 5!
    720, 5040, 40320, 362880, 3628800, // 6! ~ 10!
    39916800, 479001600, 6227020800, 87178291200, 1307674368000, // 11! ~ 15!
    20922789888000, 355687428096000, 6402373705728000, 121645100408832000, 2432902008176640000 // 16! ~ 20!
};

/*
 * 1 / n!, n in [0, 128]
 */
static f64 const factorial_reciprocal_table[] =
{
    1.0, 1.0, 0.5, 0.166666666666666666667, 0.0416666666666666666667,
    0.00833333333333333333333, 0.00138888888888888888889, 0.000198412698412698412698, 0.0000248015873015873015873, 0.00000275573192239858906526,
    2.75573192239858906526e-7, 2.50521083854417187751e-8, 2.08767569878680989792e-9, 1.60590438368216145994e-10, 1.14707455977297247139e-11,
    7.6471637318198164759e-13, 4.77947733238738529744e-14, 2.8114572543455207632e-15, 1.56192069685862264622e-16, 8.22063524662432971696e-18,
    4.11031762331216485848e-19, 1.95729410633912612308e-20, 8.89679139245057328675e-22, 3.86817017063068403772e-23, 1.61173757109611834905e-24,
    6.44695028438447339619e-26, 2.47959626322479746007e-27, 9.18368986379554614843e-29, 3.27988923706983791015e-30, 1.13099628864477169316e-31,
    3.76998762881590564385e-33, 1.21612504155351794963e-34, 3.80039075485474359259e-36, 1.15163356207719502806e-37, 3.38715753552116184723e-39,
    9.67759295863189099209e-41, 2.68822026628663638669e-42, 7.26546017915307131538e-44, 1.9119632050402819251e-45, 4.90246975651354339769e-47,
    1.22561743912838584942e-48, 2.98931082714240451079e-50, 7.1174067312914393114e-52, 1.6552108677421951887e-53, 3.7618428812322617925e-55,
    8.35965084718280398332e-57, 1.81731540156147912681e-58, 3.86662851396059388683e-60, 8.05547607075123726423e-62, 1.64397470831657903352e-63,
    3.28794941663315806703e-65, 6.44695964045717268045e-67, 1.2397999308571485924e-68, 2.3392451525606577215e-70, 4.33193546770492170648e-72,
    7.87624630491803946633e-74, 1.4064725544496499047e-75, 2.46749570956078930649e-77, 4.25430294751860225258e-79, 7.21068296189593602132e-81,
    1.20178049364932267022e-82, 1.97013195680216831184e-84, 3.17763218839059405135e-86, 5.04386061649300643071e-88, 7.88103221327032254798e-90,
    1.212466494349280392e-91, 1.83707044598375816969e-93, 2.74189618803545995477e-95, 4.0322002765227352276e-97, 5.84376851669961627188e-99,
    8.34824073814230895983e-101, 1.17580855466793083941e-102, 1.63306743703879283252e-104, 2.23707868087505867468e-106, 3.02307929847980901984e-108,
    4.03077239797307869312e-110, 5.30364789206984038568e-112, 6.88785440528550699439e-114, 8.83058257087885512102e-116, 1.11779526213656393937e-117,
    1.39724407767070492421e-119, 1.72499268848235175829e-121, 2.10364962010042897352e-123, 2.53451761457883008858e-125, 3.01728287449860724831e-127,
    3.54974455823365558624e-129, 4.12760995143448323982e-131, 4.74437925452239452853e-133, 5.39134006195726650969e-135, 6.05768546287333315695e-137,
    6.73076162541481461884e-139, 7.39644134660968639433e-141, 8.03961015935835477644e-143, 8.64474210683694061983e-145, 9.19653415620951129769e-147,
    9.68056226969422241862e-149, 1.00839190309314816861e-150, 1.03957928153932800887e-152, 1.06079518524421225395e-154, 1.07151028812546692318e-156,
    1.07151028812546692318e-158, 1.06090127537174942889e-160, 1.04009928958014649892e-162, 1.00980513551470533875e-164, 9.70966476456447441109e-167,
    9.24729977577568991533e-169, 8.72386771299593388238e-171, 8.15314739532330269382e-173, 7.54921055122528027205e-175, 6.92588123965622043308e-177,
    6.29625567241474584825e-179, 5.67230240758085211554e-181, 5.06455572105433224602e-183, 4.48190771774719667789e-185, 3.93149799802385673499e-187,
    3.41869391132509281304e-189, 2.947149923556114494e-191, 2.51893155859496965299e-193, 2.13468776152116072287e-195, 1.79385526178248800241e-197,
    1.49487938481874000201e-199, 1.23543750811466115869e-201, 1.01265369517595176942e-203, 8.23295687134920137737e-206, 6.63948134786225917529e-208,
    5.31158507828980734024e-210, 4.215543712928418524e-212, 3.3193257582113531685e-214, 2.59322324860261966289e-216
};

u64 factorial(u64 n)
{
    if (n < 21)
    {
        return factorial_table[n];
    }

    // 21! will overflow u64
    return 0;
}

f64 factorial_reciprocal(u64 n)
{
    if (n < 129)
    {
        return factorial_reciprocal_table[n];
    }

    return 0;
}

/*
 * Calculates the natural logarithm of the gamma function for a real number: ln[Г(x)]
 * Ref: computation of special functions
 */
__device__ static C64 ln_gamma_f64(f64 x)
{
    // z is zero or negative integer
    if (x == ((i64)x) && x <= 0.0)
    {
#ifdef __NVCC__
        return make_cuDoubleComplex(F64_QUASI_INFINITY, 0.0);
#else
        return F64_QUASI_INFINITY;
#endif
    }

    u64 negative = 0;

    // if x < 0, then x = -x
    if (x < 0.0)
    {
        negative = 1;
        x = -x;
    }

    f64 x_plus_n = x;
    i64 n = 0;
    if (x <= 7.0)
    {
        n = (i64)(7 - x);
        x_plus_n = x + n;
    }

    /*
     * Calculates ln[Г(x)] with eq 3.1.16 in "computation of special functions"
     * https://en.wikipedia.org/wiki/Bernoulli_number
     */
    f64 result_real = (x_plus_n - 0.5) * log(x_plus_n) - x_plus_n + 0.5 * LOG_2PI;
    static f64 BERNOULLI_COEFS[10] = {
        8.333333333333333E-02, -2.777777777777778E-03, 7.936507936507937E-04, -5.952380952380952E-04, 8.417508417508418E-04,
        -1.917526917526918E-03, 6.410256410256410E-03, -2.955065359477124E-02, 1.796443723688307E-01, -1.392432216905900E+00};
    for (i64 k = 0; k < 10; ++k)
    {
        /*
         * Note: in computation of special functions, eq 3.1.16, k starts at 1
         *  1 - 2 * (K + 1) = -1 - 2 * K
         */
        result_real += BERNOULLI_COEFS[k] * pow(x_plus_n, -1 - 2 * k);
    }

    if (x <= 7.0)
    {
        /*
         * Computation of special functions, eq 3.1.8
         * Here we use the log version, ln[Г(x)] = ln[Г(x+1)] - ln(x)
         */
        for (i64 j = 0; j < n; ++j)
        {
            result_real -= log(x + j);
        }
    }

    f64 result_imag = 0.0;
    if (negative)
    {
        /*
         * Computation of special functions, eq 3.1.10
         */
        f64 sin_x_pi = sin(M_PI * x);

        result_real = log(M_PI / (x * fabs(sin_x_pi))) - result_real;

        if (sin_x_pi > 0)
        {
            result_imag = -M_PI;
        }
    }

#ifdef __NVCC__
    return make_cuDoubleComplex(result_real, result_imag);
#else
    return result_real + result_imag * I;
#endif
}

/*
 * This is the factorial with every second value skipped.
 * For example, 7!! = 7 * 5 * 3 * 1
 */
__device__ i64 double_factorial(i64 n)
{
    if (n < -1)
    {
        return 0;
    }
    if (n <= 0)
    {
        return 1;
    }
    i64 result = 1;
    for (i64 k = n; k > 0; k -= 2)
    {
        result *= k;
    }
    return result;
}

/*
 * Double_factorial can be approximated numerically as
 * x!! = 2^{x/2} \left(\frac{\pi}{2}\right)^{(\cos(\pi x)-1)/4}\Gamma\left(\frac{x}{2}+1\right).
 * ref: https://docs.scipy.org/doc/scipy/reference/generated/scipy.special.factorial2.html
 * ref: https://mpmath.org/doc/0.19/functions/gamma.html
 */
__device__ f64 approximate_double_factorial(i64 n)
{
    if (n < 10)
    {
        return (f64)double_factorial(n);
    }
    f64 nd2 = (f64)n / 2.0;
#ifdef __NVCC__
    f64 result = cuCreal(cuCexp(ln_gamma_f64(nd2 + 1)));
#else
    f64 result = creal(cexp(ln_gamma_f64(nd2 + 1)));
#endif

    if (n % 2 == 0)
    {
        return result * pow(2, nd2);
    }
    else
    {
        return result / sqrt(M_PI) * pow(2, nd2 + 0.5);
    }
}

/*
 * Kummer confluent hypergeometric function M(a, b, x) with
 * b != 0,-1,-2 ... (non-positive integers)
 * Ref: https://en.wikipedia.org/wiki/Confluent_hypergeometric_function
 * Ref: computation of special functions, figure 12.10, p397
 * Ref: NIST handbook of mathematical functions
 */
__device__ static f64 hypergeometric1f1_b_ne_npint(f64 a, f64 b, f64 x)
{
    f64 a_original = a;
    f64 x_original = x;
    f64 result = 0.0;
    i64 n_base_cases = 1;
    i64 recurrence_count = 0;

    // Ref: NIST handbook of mathematical functions, eq 13.2.39
    if (x < 0.0)
    {
        a = b - a;
        a_original = a;
        x = -x;
    }

    if (a >= 2.0)
    {
        n_base_cases = 2;
        recurrence_count = (i64)a;
        a = a - recurrence_count - 1.0;
    }

    f64 result_1 = 0.0;
    f64 result_2 = 0.0;
    for (i64 n = 0; n < n_base_cases; ++n)
    {
        if (a_original >= 2.0)
        {
            a += 1.0;
        }
        /*
         * Ref: computation of special functions
         * Apply eq 12.7.7 when x <= (30 + |b|) or a < 0.0
         */
        if (x <= (30.0 + fabs(b)) || a < 0.0)
        {
            // Ref: NIST handbook of mathematical functions, eq 13.2.2
            result = 1.0;
            f64 hypergeometric_term = 1.0;
            for (i64 j = 0; j < 500; ++j)
            {
                hypergeometric_term *= (a + j) * x / ((b + j) * (j + 1));
                result += hypergeometric_term;

                if (result != 0.0 && fabs(hypergeometric_term / result) < 1.0E-15)
                {
                    if (x_original < 0.0)
                    {
                        // Ref: NIST handbook of mathematical functions, eq 13.2.39
                        result *= exp(x_original);
                    }
                    break;
                }
            }
        }
        else
        {
            // Ref: NIST handbook of mathematical functions, eq 13.7.2
            C64 lg_a = ln_gamma_f64(a);
            C64 lg_b = ln_gamma_f64(b);
            C64 lg_ba = ln_gamma_f64(b - a);

            f64 multiplicative_accumulator_1 = 1.0;
            f64 multiplicative_accumulator_2 = 1.0;
            f64 term_1 = 1.0;
            f64 term_2 = 1.0;
            for (i64 i = 1; i < 9; ++i)
            {
                multiplicative_accumulator_1 *= -(b - a + i - 1.0) * (a - i) / (x * i);
                term_1 += multiplicative_accumulator_1;
                multiplicative_accumulator_2 *= -(a + i - 1.0) * (a - b + i) / (x * i);
                term_2 += multiplicative_accumulator_2;
            }

            if (x_original >= 0.0)
            {
#ifdef __NVCC__
                result = cuCreal(cuCexp(cuCadd(cuCsub(lg_b, lg_a), make_cuDoubleComplex(x, 0)))) * pow(x, a - b) * term_1 +
                         cuCreal(cuCexp(cuCsub(lg_b, lg_ba))) * pow(x, -a) * cos(M_PI * a) * term_2;
#else
                result = creal(cexp(lg_b - lg_a + x)) * pow(x, a - b) * term_1 +
                         creal(cexp(lg_b - lg_ba)) * pow(x, -a) * cos(M_PI * a) * term_2;
#endif
            }
            else
            {
#ifdef __NVCC__
                result = cuCreal(cuCexp(cuCsub(lg_b, lg_a))) * pow(x, a - b) * term_1 +
                         cuCreal(cuCexp(cuCadd(cuCsub(lg_b, lg_ba), make_cuDoubleComplex(x_original, 0)))) * pow(x, -a) * cos(M_PI * a) * term_2;
#else
                result = creal(cexp(lg_b - lg_a)) * pow(x, a - b) * term_1 +
                         creal(cexp(lg_b - lg_ba + x_original)) * pow(x, -a) * cos(M_PI * a) * term_2;
#endif
            }
        }

        if (n == 0)
        {
            result_1 = result;
        }
        if (n == 1)
        {
            result_2 = result;
        }
    }

    if (a_original >= 2.0)
    {
        /*
         * Ref: NIST handbook of mathematical functions, eq 13.3.1
         * Calculates M(a+1, b, x) based on M(a-1, b, x) and M(a, b, x)
         * result_1 = M(a-1, b, x)
         * result_2 = M(a, b, x)
         */
        for (i64 i = 1; i < recurrence_count; ++i)
        {
            result = ((b - a) * result_1 + (2.0 * a - b + x) * result_2) / a;
            result_1 = result_2;
            result_2 = result;
            a += 1.0;
        }
    }
    return result;
}

/*
 * Kummer confluent hypergeometric function M(a, b, x) with a > 0 and b > 0
 */
__device__ static f64 hypergeometric1f1_a_b_gt_0(f64 a, f64 b, f64 x)
{
    // Ref: NIST handbook of mathematical functions, eq 13.2.2
    if (x == 0)
    {
        return 1;
    }

    if (a == b)
    {
        // Ref: NIST handbook of mathematical functions, eq 13.6.1
        return exp(x);
    }

    if (a - b == 1)
    {
        return (1 + x / b) * exp(x);
    }

    if ((a == 1) && (b == 2))
    {
        return expm1(x) / x;
    }

    if ((b > 0) && ((fabs(a) + 1) * fabs(x) < 0.9 * b))
    {
        f64 hypergeometric_term = 1;
        f64 result = 1;
        for (i64 k = 0; k < 500; ++k)
        {
            // Ref: NIST handbook of mathematical functions, eq 13.2.2
            hypergeometric_term *= (a + k) * x / (b + k) / (k + 1);
            result += hypergeometric_term;
            if (fabs(hypergeometric_term) <= DBL_EPSILON * fabs(result))
            {
                return result;
            }
        }
        return NAN;
    }

    return hypergeometric1f1_b_ne_npint(a, b, x);
}

/*
 * The Boys function of order n
 * Ref: Molecular electronic structure theory (purple book), eq 9.8.39
 */
__device__ f64 boys(f64 n, f64 x)
{
    return hypergeometric1f1_a_b_gt_0(n + 0.5, n + 1.5, -x) / (2.0 * n + 1.0);
}

#ifdef MODULE_TEST
#include <stdio.h>
#include "diagnostics.h"

#ifdef __NVCC__
__global__ static void boys_func_cuda_kernel(f64* input, f64* output, i32 count)
{
    i32 tid = threadIdx.x;
    i32 i = blockIdx.x * blockDim.x + tid;
    if (i >= count)
        return;
    output[i] = boys(input[i], 0.5);
}
#endif

int main(void)
{
#ifdef __NVCC__
    f64* input;
    f64* output;
    int count = 3;

    cudaMallocManaged(&input, sizeof(f64) * count, cudaMemAttachGlobal);
    cudaMallocManaged(&output, sizeof(f64) * count, cudaMemAttachGlobal);

    input[0] = 0.0;
    input[1] = 1.0;
    input[2] = 2.0;

    boys_func_cuda_kernel<<<1, 3>>>(input, output, count);
    cudaDeviceSynchronize();
    printf("%.16e %.16e %.16e\n", output[0], output[1], output[2]);

    cudaFree(input);
    cudaFree(output);
#else
    if (1)
    {
        printf("{");
        for (u64 i = 0; i < 21; ++i) // 20! = 2432902008176640000 and 21! will overflow u64
        {
            printf("%lu, ", factorial(i));
        }
        printf("}\n");
    }

    if (1)
    {
        for (f64 z = -10; z < 10; z += 0.323)
        {
            c64 d = ln_gamma_f64(z);
            if (cimag(d) != 0.0)
            {
                printf("ln_gamma(%.6f) = (%.16e + %.16e I)\n", z, creal(d), cimag(d));
            }
        }
    }

    printf("\n-----------------------------------------\n\n");

    if (1)
    {
        f64 value = hypergeometric1f1_a_b_gt_0(2, -0.3333333333333333, 3.25);
        f64 expected = -2.81595685692482e+03;
        print_f64_diff(value, expected, "1f1(2, -0.3333333333333333, 3.25)");

        value = hypergeometric1f1_a_b_gt_0(2.500000, 3.500000, -6.406566);
        expected = 3.11839417461496e-02;
        print_f64_diff(value, expected, "1f1(2.500000, 3.500000, -6.406566)");

        value = hypergeometric1f1_a_b_gt_0(1.0, 1.0, -6.406566);
        expected = 1.65068322668003e-03;
        print_f64_diff(value, expected, "1f1(1.0, 1.0, -6.406566)");

        expected = 6.36184564106256;
        value = hypergeometric1f1_a_b_gt_0(1.0, 2.0, 3.0);
        print_f64_diff(value, expected, "1f1(1.0, 2.0, 3.0)");

        value = hypergeometric1f1_a_b_gt_0(-1, 1, 3.143);
        expected = -2.143;
        print_f64_diff(value, expected, "1f1(-1, 1, 3.143)");

        value = hypergeometric1f1_a_b_gt_0(-1, -2, 3.143);
        expected = 2.5715;
        print_f64_diff(value, expected, "1f1(-1, -2, 3.143)");

        value = hypergeometric1f1_a_b_gt_0(0.01, 2, 0.98);
        expected = 1.0058634934131;
        print_f64_diff(value, expected, "1f1(0.01, 2, 0.98)");

        value = hypergeometric1f1_a_b_gt_0(0, 2, 0.98);
        expected = 1.0;
        print_f64_diff(value, expected, "1f1(0, 2, 0.98)");
    }

    if (1)
    {
        for (i64 i = 0; i < 30; ++i)
        {
            printf("%ld %ld %f\n", i, double_factorial(i), approximate_double_factorial(i));
        }
    }

    if (1)
    {
        for (f64 i = 0; i < 10; ++i)
        {
            printf("boys(%.3e, 0.5) = %.16e\n", i, boys(i, 0.5));
        }
    }
#endif
    return 0;
}
#endif
