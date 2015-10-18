CREATE TABLE test(
		id int,
		fl numeric(8,4),
		string text,
		boolean bool,
		dt timestamp without time zone,
		ts interval);

INSERT INTO test VALUES(1, 1.1, 'text one', true, '2015-01-27 23:06:03', '1 day 11:13:12');
INSERT INTO test VALUES(2, 12.12, 'text two', true, '2015-02-27 23:06:03', '1 day 12:13:12');
INSERT INTO test VALUES(3, 123.123, 'text three', false, '2015-03-27 23:06:03', '1 day 13:13:12');
INSERT INTO test VALUES(4, 1234.1234, 'text four', false, '2015-04-27 23:06:03', '1 day 14:13:12');

CREATE TABLE builtins(
		mbool boolean,
		mbyte smallint,
		mshort smallint,
		mint serial,
		mlong bigint,
		mfloat numeric(5, 2),
		mdouble numeric(8, 5),
		mstring text);

CREATE TABLE Converted(
		dt timestamp without time zone,
		date varchar(10),
		ts interval);
