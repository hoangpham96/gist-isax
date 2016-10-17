CREATE EXTENSION gist_isax;
/*
 * GUC parameters
 */
-- Default value of 0.5 should be reachable via SHOW
SHOW gist_isax.distance_threshold;

-- Shouldn't be able to set negative distances
SET gist_isax.distance_threshold TO -0.1;
SHOW gist_isax.distance_threshold;

-- Should be able to set threshold and see the updated value
SET gist_isax.distance_threshold TO 0.1;
SHOW gist_isax.distance_threshold;

/*
 * array_dist function
 */
-- Don't allow comparisons between arrays of different lengths
SELECT array_dist('{1.0,1.0,1.0,1.0}'::real[], '{2.0,2.0,2.0}'::real[]);
SELECT array_dist('{1.0,1.0,1.0}'::real[], '{2.0,2.0,2.0,2.0}'::real[]);

-- Comparisons involving NULLs should give result unknown
SELECT array_dist('{null,1.0,1.0,1.0}'::real[], '{2.0,2.0,2.0,2.0}'::real[]);
SELECT array_dist(null::real[], '{2.0,2.0,2.0,2.0}'::real[]);
SELECT array_dist('{1.0,1.0,1.0}'::real[], '{null,2.0,2.0,2.0}'::real[]);
SELECT array_dist('{1.0,1.0,1.0}'::real[], null::real[]);

-- sqrt((1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2) = 0.0
SELECT array_dist('{1.0,1.0,1.0,1.0}'::real[], '{1.0,1.0,1.0,1.0}'::real[]);

-- sqrt((1 - 2)^2 + (1 - 2)^2 + (1 - 2)^2 + (1 - 2)^2) = 2.0
SELECT array_dist('{1.0,1.0,1.0,1.0}'::real[], '{2.0,2.0,2.0,2.0}'::real[]);

-- distances should be symmetric
SELECT array_dist('{2.0,2.0,2.0,2.0}'::real[], '{1.0,1.0,1.0,1.0}'::real[]);

-- Trivial distance 0.1
SELECT array_dist('{0,0,0}'::real[], '{0,0,0.1}'::real[]);
SELECT array_dist('{0,0,0.1}'::real[], '{0,0,0}'::real[]);

/*
 * arrays_similar function
 */
SET gist_isax.distance_threshold TO 0.2;

-- Comparisons involving NULLs should give result unknown
SELECT arrays_similar('{null,1.0,1.0,1.0}'::real[], '{2.0,2.0,2.0,2.0}'::real[]);
SELECT arrays_similar(null::real[], '{2.0,2.0,2.0,2.0}'::real[]);
SELECT arrays_similar('{1.0,1.0,1.0,1.0}'::real[], '{null,2.0,2.0,2.0}'::real[]);
SELECT arrays_similar('{1.0,1.0,1.0,1.0}'::real[], null::real[]);

-- sqrt((1 - 2)^2 + (1 - 2)^2 + (1 - 2)^2 + (1 - 2)^2) = 2.0
SELECT arrays_similar('{2.0,2.0,2.0,2.0}'::real[], '{1.0,1.0,1.0,1.0}'::real[]);

-- Trivial distance 0.1
SELECT arrays_similar('{0,0,0}'::real[], '{0,0,0.1}'::real[]);
SELECT arrays_similar('{0,0,0.1}'::real[], '{0,0,0}'::real[]);

/*
 * '%' array similarity operator
 */
SET gist_isax.distance_threshold TO 0.2;
-- Comparisons involving NULLs should give result unknown
SELECT '{null,1.0,1.0,1.0}'::real[] % '{2.0,2.0,2.0,2.0}'::real[];
SELECT null::real[] % '{2.0,2.0,2.0,2.0}'::real[];
SELECT '{1.0,1.0,1.0,1.0}'::real[] % '{null,2.0,2.0,2.0}'::real[];
SELECT '{1.0,1.0,1.0,1.0}'::real[] % null::real[];

-- sqrt((1 - 2)^2 + (1 - 2)^2 + (1 - 2)^2 + (1 - 2)^2) = 2.0
SELECT '{2.0,2.0,2.0,2.0}'::real[] % '{1.0,1.0,1.0,1.0}'::real[];

-- Trivial distance 0.1
SELECT '{0,0,0}'::real[] % '{0,0,0.1}'::real[];
SELECT '{0,0,0.1}'::real[] % '{0,0,0}'::real[];

--Testing calc_lower_bp
SELECT calc_lower_bp(256,256);
SELECT calc_lower_bp(2,128);

--Testing calc_lower_bp
SELECT calc_upper_bp(256,256);
SELECT calc_upper_bp(2,128);


--Testing ts_to_paa
SELECT ts_to_paa('{-0.11252,-2.8272,-3.7739,-4.3498,-4.376,-3.475,-2.1814,-1.8183,-1.2505,-0.47749,-0.36381,-0.49196,-0.42186,-0.3092,-0.49594,-0.34212,-0.35534,-0.36791,-0.3165,-0.41237,-0.47167,-0.41346,-0.36462,-0.4493,-0.47142,-0.42478,-0.46252,-0.55247,-0.47538,-0.6942,-0.70187,-0.59381,-0.66068,-0.71383,-0.76981,-0.67228,-0.65368,-0.63941,-0.5593,-0.59167,-0.49322,-0.46305,-0.30164,-0.23273,-0.12505,-0.15394,-0.024357,-0.065609,0.034999,0.061935,0.071195,0.12393,0.10312,0.22523,0.12868,0.30248,0.25728,0.19635,0.17938,0.24473,0.34122,0.3282,0.40604,0.44661,0.42407,0.48151,0.47784,0.62408,0.57458,0.59801,0.56459,0.60798,0.62063,0.65625,0.68475,0.69427,0.66558,0.5758,0.63813,0.61492,0.56908,0.46858,0.44282,0.46827,0.43249,0.40796,0.41862,0.36253,0.41096,0.47167,0.37217,0.33788,0.22141,0.274,0.29866,0.26356,0.34256,0.41951,0.58661,0.86062,1.1733,1.2582,1.4338,1.7005,1.999,2.1253,1.9933,1.9322,1.7974,1.5223,1.2512,0.99873,0.48372,0.023132,-0.19491,-0.22092,-0.24374,-0.25469,-0.29114,-0.25649,-0.22787,-0.32242,-0.28929,-0.31817,-0.36365,-0.39346,-0.26642,-0.25682,-0.28869,-0.16234,0.16035,0.79217,0.93354,0.79696,0.57862,0.25774,0.22808,0.12343,0.92529,0.19314}
'::real[]);

--Testing paa_to_isax
SELECT paa_to_isax('{-2.46421,-0.387701,-0.477982,-0.655634,-0.176266,0.183238,0.470216,0.63229,0.445298,0.397698,1.69353,0.129489,-0.288913,0.498932}'::real[]);
