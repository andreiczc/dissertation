package ro.dissertation.dbapp.model;

import lombok.Data;

import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;

@Data
@Entity
public class IotInstance {

    @Id
    private int instanceId;

    @ManyToOne
    @JoinColumn(name = "object_id")
    private IotObject object;

    @ManyToOne
    @JoinColumn(name = "machine_id")
    private Machine machine;
}
