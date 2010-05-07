use ##database##
go
set nocount on

drop table authors
go
drop table publishers
go
drop table roysched
go
drop table sales
go
drop table salesdetail
go
drop table titleauthor
go
drop table titles
go
drop table stores
go
drop table discounts
go
drop table au_pix
go
drop table blurbs
go
drop view titleview
go
drop procedure history_proc
go
drop procedure discount_proc
go
drop procedure storename_proc
go
drop procedure storeid_proc
go
drop procedure insert_sales_proc
go
drop procedure insert_salesdetail_proc
go
drop procedure title_proc
go
drop procedure titleid_proc
go
drop procedure byroyalty
go
drop procedure waitSomeSeconds
go
sp_droptype id
go
sp_droptype tid
go
drop default typedflt
go
drop default datedflt
go
drop default phonedflt
go
drop rule title_idrule
go
drop rule pub_idrule
go
