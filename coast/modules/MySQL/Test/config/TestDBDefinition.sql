-- Definition for the test data base

DROP Database IF EXISTS mySQLTest;
Create Database mySQLTest;
use mySQLTest;

Create TABLE TestTable (	
	Name VARCHAR(10) NOT NULL PRIMARY KEY,
	Number INT UNSIGNED NOT NULL
);

INSERT INTO TestTable VALUES ("Test",1);
INSERT INTO TestTable VALUES ("MySQL",2);
INSERT INTO TestTable VALUES ("Now",33);

GRANT SELECT,INSERT,UPDATE,DELETE
ON mySQLTest.TestTable
TO TestUser@zaphod.zueri.hsr.ch IDENTIFIED BY 'allTests'
, TestUser2@zaphod.zueri.hsr.ch IDENTIFIED BY 'allTests'
