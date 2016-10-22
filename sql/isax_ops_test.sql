
CREATE TABLE isax_elem_test (isaxvalue isaxelem);

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:1') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:2') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('2:1') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('2:255') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:256') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:257') RETURNING isaxvalue;

CREATE TABLE isax_word_test (isaxword isax );

INSERT INTO isax_word_test(isaxword) VALUES ('{1:1}') RETURNING isaxword;

INSERT INTO isax_word_test(isaxword) VALUES ('{1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1}') RETURNING isaxword;

INSERT INTO isax_word_test(isaxword) VALUES ('{1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1,1:1}') RETURNING isaxword;

-- \copy test_ecg(signal) from 'data/ECG5000_TRAIN.data'
-- \copy test_ecg(signal) from 'data/ECG5000_TRAIN.data'
-- \copy test_ecg(signal) from 'data/ECG5000_TRAIN.data'

SET gist_isax.distance_threshold TO 1.2;

VACUUM ANALYZE;
-- EXPLAIN ANALYZE
SELECT t1.id, t2.id
FROM test_ecg t1, test_ecg t2
WHERE t1.signal % t2.signal AND t1.id < t2.id;
VACUUM ANALYZE;

SELECT count(t1.id)
FROM test_ecg t1, test_ecg t2
WHERE t1.signal % t2.signal AND t1.id < t2.id;
