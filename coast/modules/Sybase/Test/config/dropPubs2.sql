/* empty file */
use master
go
set nocount on

print 'Removing the "pubs2" database'
if exists (select * from master.dbo.sysdatabases
		where name = "pubs2")
begin
	drop database pubs2
end
go
