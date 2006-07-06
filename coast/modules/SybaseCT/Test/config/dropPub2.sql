use master
go
set nocount on

print 'Removing the "pub2" database'
if exists (select * from master.dbo.sysdatabases
		where name = "pub2")
begin
	drop database pub2
end
go
