create database secureiot;
use secureiot;

create table iot_instance (id bigint not null auto_increment, instance_id integer not null, machine_id varchar(255), object_id integer, primary key (id)) engine=InnoDB;
create table iot_object (object_id integer not null, friendly_name varchar(255), primary key (object_id)) engine=InnoDB;
create table iot_record (id bigint not null auto_increment, datatype varchar(255), timestamp datetime(6), value varchar(255), resource_id integer, primary key (id)) engine=InnoDB;
create table iot_resource (resource_id integer not null, friendly_name varchar(255), object_id integer, primary key (resource_id)) engine=InnoDB;
create table machine (mac_address varchar(255) not null, friendly_name varchar(255), last_attestation datetime(6), primary key (mac_address)) engine=InnoDB;
alter table iot_instance add constraint FKkteeel2tweqoc72xoie736va8 foreign key (machine_id) references machine (mac_address);
alter table iot_instance add constraint FKf5lekf4gapuiff5g2ghqmt4dm foreign key (object_id) references iot_object (object_id);
alter table iot_record add constraint FKmk6ee2wy53sxp8k2q1x7no2pc foreign key (resource_id) references iot_resource (resource_id);
alter table iot_resource add constraint FKrkpcnjhnvjp9ga1kgi3nxeoyt foreign key (object_id) references iot_object (object_id);
