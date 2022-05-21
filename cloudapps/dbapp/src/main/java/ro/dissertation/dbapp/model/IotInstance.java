package ro.dissertation.dbapp.model;

import lombok.Data;

import javax.persistence.*;

@Data
@Entity
public class IotInstance {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private long id;

    private int instanceId;

    @ManyToOne
    @JoinColumn(name = "object_id")
    private IotObject object;

    @ManyToOne
    @JoinColumn(name = "machine_id")
    private Machine machine;
}
