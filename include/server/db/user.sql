create table user(
    id int primary key auto_increment,
    name varchar(50) not null unique,
    password varchar(50) not null,
    state enum("online","offline") default "offline"
);