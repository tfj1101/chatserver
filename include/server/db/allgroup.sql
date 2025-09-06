create table allgroup(
    id int not null auto_increment,
    groupname varchar(50) not null ,
    groupdesc varchar(200) not null default '',
    primary key(id)
);