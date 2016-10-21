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
SELECT calc_lower_bp(0,256);
SELECT calc_lower_bp(255,256);
SELECT calc_lower_bp(2,128);

--Testing calc_lower_bp
SELECT calc_upper_bp(0,256);
SELECT calc_upper_bp(255,256);
SELECT calc_upper_bp(2,128);

--Copy ECG5000 data to table test_ecg
CREATE TABLE test_ecg(id SERIAL PRIMARY KEY, signal real[]);
\copy test_ecg(signal) from 'data/ECG5000_TRAIN.data'

--Testing ts_to_paa
SELECT ts_to_paa(signal) FROM test_ecg WHERE id = 1;

--Create table for PAA representation of the signals
CREATE TABLE test_ecg_paa(id SERIAL PRIMARY KEY, signal_paa real[]);
INSERT INTO test_ecg_paa(signal_paa) SELECT ts_to_paa(signal) FROM test_ecg;

--Testing paa_to_isax
SELECT paa_to_isax(signal_paa) FROM test_ecg_paa WHERE id =1 ;


--Testing isax
CREATE TABLE isax_test(id SERIAL PRIMARY KEY, signal_isax isax);
-- INSERT INTO isax_test(signal_isax) values (cast((SELECT paa_to_isax(signal_paa) FROM test_ecg_paa) as cstring));
INSERT INTO isax_test(signal_isax) VALUES ('{1:256,89:256,80:256,65:256,110:256,146:256,174:256,188:256,172:256,167:256,244:256,141:256,98:256,176:256}');
SELECT * FROM isax_test;

--Testing penalty_implementation. Should be -(14 * 128) = -1792
SELECT penalty_implementation('{1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256,1:256}','{1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128,1:128}');

--Testing union_implementation. Should be arrays of 127:128 and 63:64
SELECT union_implementation('{254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256}','{255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256,255:256}');
SELECT union_implementation('{254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256,254:256}','{126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128,126:128}');

--Testing mindist
SELECT mindist('{1:256,89:256,80:256,65:256,110:256,146:256,174:256,188:256,172:256,167:256,244:256,141:256,98:256,176:256}','{-2.46421,-0.387701,-0.477982,-0.655634,-0.176266,0.183238,0.470216,0.63229,0.445298,0.397698,1.69353,0.129489,-0.288913,0.498932}'::real[]);
