CREATE TABLE isax_elem_test (id INTEGER PRIMARY KEY, isaxvalue isaxelem);

INSERT INTO isax_elem_test VALUES (1, '1:1');
SELECT isaxvalue FROM isax_elem_test WHERE id=1;