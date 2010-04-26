-- Definition for the test data base

DROP Database IF EXISTS coast_tests;
Create Database coast_tests;
use coast_tests;

Create TABLE TestTable (	
	Name VARCHAR(10) NOT NULL PRIMARY KEY,
	Number INT UNSIGNED NOT NULL
);

INSERT INTO TestTable VALUES ("Test",1);
INSERT INTO TestTable VALUES ("MySQL",2);
INSERT INTO TestTable VALUES ("Now",33);

GRANT SELECT,INSERT,UPDATE,DELETE
ON coast_tests.TestTable
TO 'TestUser'@'localhost' IDENTIFIED BY 'CoastMySQL10';

GRANT SELECT,INSERT,UPDATE,DELETE
ON coast_tests.TestTable
TO 'TestUser'@'sifs-coast1.hsr.ch' IDENTIFIED BY 'CoastMySQL10';

