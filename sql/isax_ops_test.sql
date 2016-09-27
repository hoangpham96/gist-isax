CREATE TABLE isax_elem_test (isaxvalue isaxelem);

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:1') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:2') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('2:1') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('2:255') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:256') RETURNING isaxvalue;

INSERT INTO isax_elem_test(isaxvalue) VALUES ('1:257') RETURNING isaxvalue;
