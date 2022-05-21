package ro.dissertation.dbapp.model;

import lombok.Data;

import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;

@Data
@Entity
public class IotResource {

    @Id
    private int resourceId;

    private String friendlyName;

    @ManyToOne
    @JoinColumn(name = "object_id")
    private IotObject object;
}
