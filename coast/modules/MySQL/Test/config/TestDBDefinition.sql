-- Definition for the test data base

DROP Database IF EXISTS ##database##;
Create Database ##database##;
use ##database##;

Create TABLE TestTable (
	Name VARCHAR(10) NOT NULL PRIMARY KEY,
	Number INT UNSIGNED NOT NULL
);

INSERT INTO TestTable VALUES ("Test",1);
INSERT INTO TestTable VALUES ("MySQL",2);
INSERT INTO TestTable VALUES ("Now",33);

GRANT SELECT,INSERT,UPDATE,DELETE
ON ##database##.TestTable
TO '##username##'@'localhost' IDENTIFIED BY '##password##';

GRANT SELECT,INSERT,UPDATE,DELETE
ON ##database##.TestTable
TO '##username##'@'##server##' IDENTIFIED BY '##password##';

