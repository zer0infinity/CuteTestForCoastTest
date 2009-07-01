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
--   DATA FOR TABLE BONUS
--   FILTER = none used
---------------------------------------------------
REM INSERTING into BONUS

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
--  DDL for Procedure GETEMPSALARY
--------------------------------------------------------
set define off;

  CREATE OR REPLACE PROCEDURE "GETEMPSALARY" 
( id IN NUMBER
, salary OUT emp.sal%type
) AS
BEGIN
  select emp.sal into salary from emp where emp.empno=id;
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

