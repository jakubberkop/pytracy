# First Iteration
See gretl.gdt for the data and script.

OPTIONS = [
	[ "-DPYTRACY_CONSTANT_FUNCTION_DATA", "-DPYTRACY_CONSTANT_FUNCTION_DATA_DISABLED"],
	[ "-DPYTRACY_FILTERING_OLD", "-DPYTRACY_FILTERING_ATOMIC", "-DPYTRACY_FILTERING_GENERATIONS" ],
	[ "-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION", "-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION_DISABLED" ],
]

P confidenc level: 0.05


Model 9: OLS, using observations 1-72
Dependent variable: time
Omitted because all values were zero: C_F_DATA_DISABLED DEDU_FUNCTION_DATA_DISABLED
Omitted due to exact collinearity: FILTERING_ATOMIC

                      coefficient   std. error    t-ratio    p-value 
  -------------------------------------------------------------------
  const                3.38496      0.0409331      82.69     2.22e-68 ***
  ID                   0.000482383  0.000634124     0.7607   0.4495  
  C_F_DATA             0.000273291  0.0228299       0.01197  0.9905  
  FILTERING_OLD       −0.00379542   0.0305521      −0.1242   0.9015  
  DEDU_FUNCTION_DA~   −3.19946      0.0224839    −142.3      7.55e-84 ***
  FILTERING_GENERA~   −0.0414869    0.0303404      −1.367    0.1761  

Mean dependent var   1.787394   S.D. dependent var   1.612486
Sum squared resid    0.588619   S.E. of regression   0.094438
R-squared            0.996812   Adjusted R-squared   0.996570
F(5, 66)             4126.697   P-value(F)           6.23e-81
Log-likelihood       70.87553   Akaike criterion    −129.7511
Schwarz criterion   −116.0911   Hannan-Quinn        −124.3130

Excluding the constant, p-value was highest for variable 2 (C_F_DATA)


White's test for heteroskedasticity
OLS, using observations 1-72
Dependent variable: uhat^2

                      coefficient   std. error   t-ratio   p-value
  ----------------------------------------------------------------
  const                0.0326800    0.0428735     0.7622   0.4491 
  ID                  −0.000298662  0.00105444   −0.2832   0.7780 
  C_F_DATA            −0.0146464    0.0244816    −0.5983   0.5521 
  FILTERING_OLD       −0.0136734    0.0415641    −0.3290   0.7434 
  DEDU_FUNCTION_DA~   −0.0125953    0.0191313    −0.6584   0.5130 
  FILTERING_GENERA~   −0.0207489    0.0402441    −0.5156   0.6082 
  sq_ID                2.84974e-06  1.00366e-05   0.2839   0.7775 
  X2_X3               −1.57871e-05  0.000334971  −0.04713  0.9626 
  X2_X4                0.000185975  0.000740842   0.2510   0.8027 
  X2_X5               −0.000124979  0.000336894  −0.3710   0.7121 
  X2_X6                9.11226e-05  0.000844835   0.1079   0.9145 
  X3_X4                0.0167637    0.0212445     0.7891   0.4334 
  X3_X5                0.00340779   0.00940648    0.3623   0.7185 
  X3_X6                0.0144877    0.0218069     0.6644   0.5092 
  X4_X5               −0.00616909   0.0117839    −0.5235   0.6027 
  X5_X6                0.00811428   0.0160980     0.5041   0.6162 

  Unadjusted R-squared = 0.341879



Test statistic: TR^2 = 24.615260,
with p-value = P(Chi-square(15) > 24.615260) = 0.055355


No heteroskedasticity detected

Result:
The model is not heteroskedastic.
DEDU_FUNCTION_DATA is the most significant variable in the model.

Following analysis will only include samples with DEDU_FUNCTION_DATA = 1


Model 11: OLS, using observations 1-36
Dependent variable: time
Omitted because all values were zero: C_F_DATA_DISABLED
Omitted due to exact collinearity: FILTERING_ATOMIC

                      coefficient   std. error   t-ratio  p-value 
  ----------------------------------------------------------------
  const                0.194067     0.00250169   77.57    4.51e-37 ***
  ID                  −5.40965e-05  3.87047e-05  −1.398   0.1721  
  C_F_DATA            −0.000855972  0.00131264   −0.6521  0.5191  
  FILTERING_OLD       −0.00464357   0.00203954   −2.277   0.0299   **
  FILTERING_GENERA~   −0.00349152   0.00164862   −2.118   0.0423   **

Mean dependent var   0.188872   S.D. dependent var   0.004032
Sum squared resid    0.000470   S.E. of regression   0.003895
R-squared            0.173396   Adjusted R-squared   0.066738
F(4, 31)             1.625716   P-value(F)           0.192689
Log-likelihood       151.3412   Akaike criterion    −292.6823
Schwarz criterion   −284.7647   Hannan-Quinn        −289.9189

Excluding the constant, p-value was highest for variable 2 (C_F_DATA)

White's test for heteroskedasticity
OLS, using observations 1-36
Dependent variable: uhat^2

                      coefficient   std. error    t-ratio   p-value
  -----------------------------------------------------------------
  const                1.01266e-05  0.000153906   0.06580   0.9481 
  ID                   2.30445e-08  3.56733e-06   0.006460  0.9949 
  C_F_DATA             7.83208e-06  0.000111974   0.06995   0.9448 
  FILTERING_OLD        4.50453e-06  0.000147458   0.03055   0.9759 
  FILTERING_GENERA~   −2.30482e-05  0.000139745  −0.1649    0.8703 
  sq_ID                5.79024e-10  3.10714e-08   0.01864   0.9853 
  X2_X3                3.27023e-07  1.56329e-06   0.2092    0.8360 
  X2_X4                1.86875e-08  2.36995e-06   0.007885  0.9938 
  X2_X5                7.69289e-07  2.32269e-06   0.3312    0.7432 
  X3_X4               −2.33930e-05  9.77021e-05  −0.2394    0.8127 
  X3_X5               −5.67354e-05  8.73939e-05  −0.6492    0.5221 

  Unadjusted R-squared = 0.194215

Test statistic: TR^2 = 6.991756,
with p-value = P(Chi-square(10) > 6.991756) = 0.726223

Result:
No heteroskedasticity detected.
ID and C_F_DATA are not significant in the model.

Comparing filtering: OLD, GENERATIONS AND ATOMIC

Model 15: OLS, using observations 1-36
Dependent variable: time

                      coefficient   std. error   t-ratio   p-value 
  -----------------------------------------------------------------
  const                0.187927     0.00112702  166.7      7.22e-50 ***
  FILTERING_GENERA~   −2.45571e-05  0.00159385   −0.01541  0.9878  
  FILTERING_ATOMIC     0.00285838   0.00159385    1.793    0.0821   *

Mean dependent var   0.188872   S.D. dependent var   0.004032
Sum squared resid    0.000503   S.E. of regression   0.003904
R-squared            0.115885   Adjusted R-squared   0.062302
F(2, 33)             2.162721   P-value(F)           0.131038
Log-likelihood       150.1304   Akaike criterion    −294.2609
Schwarz criterion   −289.5103   Hannan-Quinn        −292.6028

White's test for heteroskedasticity
OLS, using observations 1-36
Dependent variable: uhat^2

                      coefficient  std. error   t-ratio  p-value
  --------------------------------------------------------------
  const               7.38707e-06  6.86864e-06  1.075    0.2900 
  FILTERING_GENERA~   3.24639e-06  9.71372e-06  0.3342   0.7403 
  FILTERING_ATOMIC    1.65084e-05  9.71372e-06  1.699    0.0986  *

  Unadjusted R-squared = 0.089470

Test statistic: TR^2 = 3.220925,
with p-value = P(Chi-square(2) > 3.220925) = 0.199795

Results:
No heteroskedasticity detected.
FILTERING_ATOMIC is significant in the model.

Results:

DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION is the most significant.
PYTRACY_CONSTANT_FUNCTION_DATA is not significant, we choose to disable it.

FILTERING was inconclusive, next iteration of tests focus on just FILTERING_ATOMIC

## Filtering options

OPTIONS = [
	[ "-DPYTRACY_CONSTANT_FUNCTION_DATA", "-DPYTRACY_CONSTANT_FUNCTION_DATA_DISABLED"],
	[ "-DPYTRACY_FILTERING_OLD", "-DPYTRACY_FILTERING_ATOMIC", "-DPYTRACY_FILTERING_GENERATIONS" ],
	[ "-", "-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION_DISABLED" ],
]

OPTIONS2 from test_flags.py

Results in results7.csv and gretl2.gdt

Model 1: OLS, using observations 1-45
Dependent variable: time

                      coefficient   std. error   t-ratio  p-value 
  ----------------------------------------------------------------
  const                0.201178     0.00233679   86.09    5.94e-48 ***
  FILTERING_ATOMIC    −0.00655421   0.00262066   −2.501   0.0165   **
  FILTERING_GENERA~   −0.00334106   0.00272353   −1.227   0.2269  
  ID                   1.20911e-05  8.56134e-05   0.1412  0.8884  

Mean dependent var   0.198146   S.D. dependent var   0.007344
Sum squared resid    0.002056   S.E. of regression   0.007081
R-squared            0.133713   Adjusted R-squared   0.070326
F(3, 41)             2.109472   P-value(F)           0.113806
Log-likelihood       161.0102   Akaike criterion    −314.0204
Schwarz criterion   −306.7937   Hannan-Quinn        −311.3264

Excluding the constant, p-value was highest for variable 1 (ID)

White's test for heteroskedasticity -
  Null hypothesis: heteroskedasticity not present
  Test statistic: LM = 3.28361
  with p-value = P(Chi-square(6) > 3.28361) = 0.772501

White's test for heteroskedasticity
OLS, using observations 1-45
Dependent variable: uhat^2

                      coefficient   std. error   t-ratio   p-value
  ----------------------------------------------------------------
  const                2.73844e-05  4.48778e-05   0.6102   0.5454 
  FILTERING_ATOMIC    −2.69102e-05  6.92448e-05  −0.3886   0.6997 
  FILTERING_GENERA~   −2.12384e-05  9.06240e-05  −0.2344   0.8160 
  ID                   4.66136e-06  4.71194e-06   0.9893   0.3288 
  X2_X4               −1.01881e-06  3.06097e-06  −0.3328   0.7411 
  X3_X4                1.59109e-07  3.75781e-06   0.04234  0.9664 
  sq_ID               −9.52511e-08  1.25852e-07  −0.7568   0.4538 

  Unadjusted R-squared = 0.072969

Test statistic: TR^2 = 3.283606,
with p-value = P(Chi-square(6) > 3.283606) = 0.772501

Results:
No heteroskedasticity detected.
FILTERING_ATOMIC is significant in the model. Provides a 2% improvement in performance.

# Conclusion
Use DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION, disable DPYTRACY_CONSTANT_FUNCTION_DATA and use DPYTRACY_FILTERING_ATOMIC

# PYTRACY_USE_SYS_PROFILING

- PYTRACY_USE_SYS_PROFILING
- 209ns  57ns
- OLD PROFILING
- 507ns 132n
