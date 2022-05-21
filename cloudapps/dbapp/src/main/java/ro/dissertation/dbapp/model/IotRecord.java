package ro.dissertation.dbapp.model;

import lombok.Data;

import javax.persistence.*;
import java.util.Date;

@Data
@Entity
public class IotRecord {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private long id;

    private String datatype;

    private String value;

    private Date timestamp;

    @ManyToOne
    @JoinColumn(name = "resource_id")
    private IotResource resource;
}
