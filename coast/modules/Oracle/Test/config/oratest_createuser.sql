drop user oratest cascade;
create user oratest identified by oratest;
grant connect to oratest;
grant resource to oratest;

drop user oratest2 cascade;
create user oratest2 identified by oratest2;
grant connect to oratest2;
grant CREATE SYNONYM to oratest2;
