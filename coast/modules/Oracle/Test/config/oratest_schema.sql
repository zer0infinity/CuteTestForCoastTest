--------------------------------------------------------
--  File created - Mittwoch-Juli-01-2009
--------------------------------------------------------

--------------------------------------------------------
--  DDL for Table BONUS
--------------------------------------------------------

  CREATE TABLE "BONUS"
   (	"ENAME" VARCHAR2(10),
	"JOB" VARCHAR2(9),
	"SAL" NUMBER,
	"COMM" NUMBER
   ) ;
--------------------------------------------------------
--  DDL for Table DEPT
--------------------------------------------------------

  CREATE TABLE "DEPT"
   (	"DEPTNO" NUMBER(2,0),
	"DNAME" VARCHAR2(14),
	"LOC" VARCHAR2(13)
   ) ;
--------------------------------------------------------
--  DDL for Table EMP
--------------------------------------------------------

  CREATE TABLE "EMP"
   (	"EMPNO" NUMBER(4,0),
	"ENAME" VARCHAR2(10),
	"JOB" VARCHAR2(9),
	"MGR" NUMBER(4,0),
	"HIREDATE" DATE,
	"SAL" NUMBER(7,2),
	"COMM" NUMBER(7,2),
	"DEPTNO" NUMBER(2,0)
   ) ;
--------------------------------------------------------
--  DDL for Table SALGRADE
--------------------------------------------------------

  CREATE TABLE "SALGRADE"
   (	"GRADE" NUMBER,
	"LOSAL" NUMBER,
	"HISAL" NUMBER
   ) ;

---------------------------------------------------
--   DATA FOR TABLE SALESDETAIL
---------------------------------------------------
	CREATE TABLE "SALESDETAIL"
	("STOR_ID" CHAR(4) NOT NULL,
	"ORD_NUM" VARCHAR(20) NOT NULL,
	"TITLE_ID" VARCHAR(6) NOT NULL,
	"QTY" SMALLINT NOT NULL,
	"DISCOUNT" FLOAT NOT NULL);

---------------------------------------------------
--   END DATA FOR TABLE BONUS
---------------------------------------------------

---------------------------------------------------
--   DATA FOR TABLE SALGRADE
--   FILTER = none used
---------------------------------------------------
REM INSERTING into SALGRADE
Insert into SALGRADE (GRADE,LOSAL,HISAL) values (1,700,1200);
Insert into SALGRADE (GRADE,LOSAL,HISAL) values (2,1201,1400);
Insert into SALGRADE (GRADE,LOSAL,HISAL) values (3,1401,2000);
Insert into SALGRADE (GRADE,LOSAL,HISAL) values (4,2001,3000);
Insert into SALGRADE (GRADE,LOSAL,HISAL) values (5,3001,9999);

---------------------------------------------------
--   END DATA FOR TABLE SALGRADE
---------------------------------------------------

---------------------------------------------------
--   DATA FOR TABLE EMP
--   FILTER = none used
---------------------------------------------------
REM INSERTING into EMP
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7369,'SMITH','CLERK',7902,to_timestamp('17.12.80 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),800,null,20);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7499,'ALLEN','SALESMAN',7698,to_timestamp('20.02.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),1600,300,30);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7521,'WARD','SALESMAN',7698,to_timestamp('22.02.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),1250,500,30);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7566,'JONES','MANAGER',7839,to_timestamp('02.04.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),2975,null,20);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7654,'MARTIN','SALESMAN',7698,to_timestamp('28.09.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),1250,1400,30);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7698,'BLAKE','MANAGER',7839,to_timestamp('01.05.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),2850,null,30);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7782,'CLARK','MANAGER',7839,to_timestamp('09.06.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),2450,null,10);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7788,'SCOTT','ANALYST',7566,to_timestamp('19.04.87 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),3000,null,20);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7839,'KING','PRESIDENT',null,to_timestamp('17.11.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),5000,null,10);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7844,'TURNER','SALESMAN',7698,to_timestamp('08.09.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),1500,0,30);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7876,'ADAMS','CLERK',7788,to_timestamp('23.05.87 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),1100,null,20);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7900,'JAMES','CLERK',7698,to_timestamp('03.12.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),950,null,30);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7902,'FORD','ANALYST',7566,to_timestamp('03.12.81 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),3000,null,20);
Insert into EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO) values (7934,'MILLER','CLERK',7782,to_timestamp('23.01.82 00:00:00.000000000','DD.MM.RR HH24:MI:SS.FF'),1300,null,10);

---------------------------------------------------
--   END DATA FOR TABLE EMP
---------------------------------------------------

---------------------------------------------------
--   DATA FOR TABLE SALESDETAIL
---------------------------------------------------
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '234518', 'TC3218', 75, 40);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '234518', 'TC7777', 75, 40);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Asoap432', 'TC3218', 50, 40);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Asoap432', 'TC7777', 80, 40);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'TC3218', 85, 40);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-A-7', 'PS3333', 90, 45);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-A-7', 'TC3218', 40, 45);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-A-7', 'PS2106', 30, 45);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-V-7', 'PS2106', 50, 45);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '55-V-7', 'PS2106', 31, 45);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-A-7', 'MC3021', 69, 45);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'BS-345-DSE-860-1F2', 'PC1035', 1000, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'BU2075', 500, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'BU1032', 200, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'BU7832', 150, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'PS7777', 125, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'TC7777', 1000, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'BU1032', 1000, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'PC1035', 750, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Fsoap867', 'BU1032', 200, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA52498', 'BU7832', 100, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA71224', 'PS7777', 200, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA71224', 'PC1035', 300, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA71224', 'TC7777', 350, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZD-123-DFG-752-9G8', 'PS2091', 1000, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'PS2091', 200, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'PS7777', 250, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'PS3333', 345, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'BU7832', 360, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'PS2091', 845, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'PS7777', 581, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZZ-999-ZZZ-999-0A0', 'PS1372', 375, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'BU1111', 175, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'BU7832', 885, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZD-123-DFG-752-9G8', 'BU7832', 900, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'TC4203', 550, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Fsoap867', 'TC4203', 350, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '234518', 'TC4203', 275, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA71224', 'TC4203', 500, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'TC4203', 512, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Fsoap867', 'MC3021', 400, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'PC8888', 105, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'PC8888', 300, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA71224', 'PC8888', 350, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-3.142', 'PC8888', 335, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Asoap432', 'BU1111', 500, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '234518', 'BU1111', 340, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'BU1111', 370, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZD-123-DFG-752-9G8', 'PS3333', 750, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-J-9', 'BU7832', 300, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-E-7', 'BU2075', 150, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-E-7', 'BU1032', 300, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-E-7', 'PC1035', 400, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-A-7', 'PS7777', 180, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-J-9', 'TC4203', 250, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-E-7', 'TC4203', 226, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '13-E-7', 'MC3021', 400, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '91-V-7', 'BU1111', 390, 51.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AB-872-DEF-732-2Z1', 'MC3021', 5000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'PC8888', 2000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'BU2075', 2000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'GH-542-NAD-713-9F9', 'PC1035', 2000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZA-000-ASD-324-4D1', 'PC1035', 2000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZA-000-ASD-324-4D1', 'PS7777', 1500, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZD-123-DFG-752-9G8', 'BU2075', 3000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZD-123-DFG-752-9G8', 'TC7777', 1500, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZS-645-CAT-415-1B2', 'BU2075', 3000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZS-645-CAT-415-1B2', 'BU2075', 3000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'PS3333', 2687, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'TC7777', 1090, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'XS-135-DER-432-8J2', 'PC1035', 2138, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZZ-999-ZZZ-999-0A0', 'MC2222', 2032, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZZ-999-ZZZ-999-0A0', 'BU1111', 1001, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZA-000-ASD-324-4D1', 'BU1111', 1100, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'BU7832', 1400, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'BS-345-DSE-860-1F2', 'TC4203', 2700, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'GH-542-NAD-713-9F9', 'TC4203', 2500, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'TC4203', 3500, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'BS-345-DSE-860-1F2', 'MC3021', 4500, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AX-532-FED-452-2Z7', 'MC3021', 1600, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'NF-123-ADS-642-9G3', 'MC3021', 2550, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZA-000-ASD-324-4D1', 'MC3021', 3000, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZS-645-CAT-415-1B2', 'MC3021', 3200, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'BS-345-DSE-860-1F2', 'BU2075', 2200, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'GH-542-NAD-713-9F9', 'BU1032', 1500, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'ZZ-999-ZZZ-999-0A0', 'PC8888', 1005, 50);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '124152', 'BU2075', 42, 50.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '124152', 'PC1035', 25, 50.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Asoap132', 'BU2075', 35, 50.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-1.142', 'PC1035', 34, 50.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-1.142', 'TC4203', 53, 50.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '12-F-9', 'BU2075', 30, 55.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '12-F-9', 'BU1032', 94, 55.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA27618', 'BU2075', 200, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7896', '124152', 'TC4203', 350, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA27618', 'TC4203', 230, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7066', 'BA27618', 'MC3021', 200, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Asoap132', 'MC3021', 137, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-1.142', 'MC3021', 270, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-1.142', 'BU2075', 230, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7131', 'Asoap132', 'BU1032', 345, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('7067', 'NB-1.142', 'BU1032', 136, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '12-F-9', 'TC4203', 300, 62.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '12-F-9', 'MC3021', 270, 62.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('8042', '12-F-9', 'PC1035', 133, 62.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AB-123-DEF-425-1Z3', 'TC4203', 2500, 60.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('5023', 'AB-123-DEF-425-1Z3', 'BU2075', 4000, 60.5);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '342157', 'BU2075', 200, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '342157', 'MC3021', 250, 57.2);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '356921', 'PS3333', 200, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '356921', 'PS7777', 500, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '356921', 'TC3218', 125, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '234518', 'BU2075', 135, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '234518', 'BU1032', 320, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '234518', 'TC4203', 300, 46.7);
Insert into SALESDETAIL (STOR_ID,ORD_NUM,TITLE_ID,QTY,DISCOUNT) values ('6380', '234518', 'MC3021', 400, 46.7);


---------------------------------------------------
--   DATA FOR TABLE DEPT
--   FILTER = none used
---------------------------------------------------
REM INSERTING into DEPT
Insert into DEPT (DEPTNO,DNAME,LOC) values (10,'ACCOUNTING','NEW YORK');
Insert into DEPT (DEPTNO,DNAME,LOC) values (20,'RESEARCH','DALLAS');
Insert into DEPT (DEPTNO,DNAME,LOC) values (30,'SALES','CHICAGO');
Insert into DEPT (DEPTNO,DNAME,LOC) values (40,'OPERATIONS','BOSTON');

---------------------------------------------------
--   END DATA FOR TABLE DEPT
---------------------------------------------------

--------------------------------------------------------
--  Constraints for Table DEPT
--------------------------------------------------------

  ALTER TABLE "DEPT" ADD CONSTRAINT "PK_DEPT" PRIMARY KEY ("DEPTNO") ENABLE;
--------------------------------------------------------
--  Constraints for Table EMP
--------------------------------------------------------

  ALTER TABLE "EMP" ADD CONSTRAINT "PK_EMP" PRIMARY KEY ("EMPNO") ENABLE;

--------------------------------------------------------
--  Ref Constraints for Table EMP
--------------------------------------------------------

  ALTER TABLE "EMP" ADD CONSTRAINT "FK_DEPTNO" FOREIGN KEY ("DEPTNO")
	  REFERENCES "DEPT" ("DEPTNO") ENABLE;

--------------------------------------------------------
--  DDL for Function GETMAXSALARY
--------------------------------------------------------

  CREATE OR REPLACE FUNCTION "GETMAXSALARY"
return number
is
  salary emp.sal%type;
begin
  select max(emp.sal) into salary from emp;
  return salary;
end;

/

--------------------------------------------------------
--  DDL for Procedure GETEMPNAME
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETEMPNAME"
( id IN NUMBER
, name OUT VARCHAR2
) AS
BEGIN
  select emp.ename into name from emp where emp.empno=id;
END GETEMPNAME;

/

--------------------------------------------------------
--  DDL for Procedure GETEMPS
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETEMPS" (EmpCursor in out SYS_REFCURSOR) is
begin
  open EmpCursor for
    select emp.ename from emp order by emp.ename;
end;

/

--------------------------------------------------------
--  DDL for Function GETEMPSFUNC
--------------------------------------------------------

  CREATE OR REPLACE FUNCTION "GETEMPSFUNC"
return SYS_REFCURSOR
is
	EmpCursor SYS_REFCURSOR;
begin
  open EmpCursor for
    select emp.ename from emp order by emp.ename;
  return EmpCursor;
end;

/

--------------------------------------------------------
--  DDL for Procedure GETEMPSALARY
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETEMPSALARY"
( id IN NUMBER
, salary OUT emp.sal%type
) AS
BEGIN
  IF id IS NULL THEN
    salary := 0;
  ELSE
    select emp.sal into salary from emp where emp.empno=id;
  END IF;
END GETEMPSALARY;

/

--------------------------------------------------------
--  DDL for Procedure GETFIRSTEMPNAME
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETFIRSTEMPNAME"
( name OUT VARCHAR2
) AS
BEGIN
  select emp.ename into name from emp where emp.empno='7839';
END GETFIRSTEMPNAME;

/

--------------------------------------------------------
--  DDL for Procedure GETFLOAT
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETFLOAT"
( result OUT FLOAT
) AS
BEGIN
  result := 3.14;
END GETFLOAT;

/

--------------------------------------------------------
--  DDL for Procedure GETMAXSALARYPROC
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETMAXSALARYPROC" (maxsal out emp.sal%type)
is
begin
  select max(emp.sal) into maxsal from emp;
end;

/

--------------------------------------------------------
--  DDL for Procedure GETSTRING
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETSTRING"
( loopcount IN number, result OUT VARCHAR2
) AS
BEGIN
  for i in 1..loopcount loop
    result := result || 'a';
  end loop;
END GETSTRING;

/

