drop table friend;

create table friend(
    userid int not null,
    friendid int not null,
    primary key(userid, friendid),
    foreign key(userid) references user(id) on delete cascade,
    foreign key(friendid) references user(id) on delete cascade,
    constraint chk_userid_friendid check (userid != friendid)
);

insert into friend values(1,2);