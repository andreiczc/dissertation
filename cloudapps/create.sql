create table record (id bigint not null auto_increment, data_type varchar(255), timestamp datetime(6), value varchar(255), sensor_id bigint not null, primary key (id)) engine=InnoDB;
create table sensor (id bigint not null auto_increment, instance_id bigint not null, object_id bigint not null, primary key (id)) engine=InnoDB;
alter table record add constraint FK6u79h06y3wcw5w05ywmtfxsdm foreign key (sensor_id) references sensor (id);
