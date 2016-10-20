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

--Copy ECG5000 data to table test_ecg
CREATE TABLE test_ecg(id SERIAL PRIMARY KEY, signal real[]);
\copy test_ecg(signal) from 'data/ECG5000_TRAIN.data'

--Testing ts_to_paa
SELECT ts_to_paa(signal) FROM test_ecg WHERE id = 1;

--Create table for PAA representation of the signals
CREATE TABLE test_ecg_paa(id SERIAL PRIMARY KEY, signal_paa real[]);
INSERT INTO test_ecg_paa(signal_paa) SELECT ts_to_paa(signal) FROM test_ecg;


SELECT signal_paa FROM test_ecg_paa WHERE id =1;

--Testing paa_to_isax
SELECT paa_to_isax(signal_paa) FROM test_ecg_paa WHERE id =1 ;


--Testing isax
CREATE TABLE isax_test(id SERIAL PRIMARY KEY, signal_isax isax);
-- INSERT INTO isax_test(signal_isax) SELECT paa_to_isax(signal_paa)::isax FROM test_ecg_paa;
INSERT INTO isax_test(signal_isax) VALUES ('{2:256,90:256,81:256,66:256,111:256,147:256,175:256,189:256,173:256,168:256,245:256,142:256,99:256,177:256}');
SELECT * FROM isax_test;
