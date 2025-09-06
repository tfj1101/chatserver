create table groupuser(
    groupid int not null,
    userid int not null,
    grouprole enum('creator','manager','normal') not null default 'normal',
    primary key(groupid, userid),
    foreign key(groupid) references allgroup(id) on delete cascade,
    foreign key(userid) references user(id) on delete cascade
);