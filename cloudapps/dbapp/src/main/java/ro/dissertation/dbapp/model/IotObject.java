package ro.dissertation.dbapp.model;

import lombok.Data;

import javax.persistence.*;

@Data
@Entity
public class IotObject {

    @Id
    private int objectId;

    private String friendlyName;
}
