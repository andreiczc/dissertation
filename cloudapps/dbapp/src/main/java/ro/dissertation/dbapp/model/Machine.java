package ro.dissertation.dbapp.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.persistence.Entity;
import javax.persistence.Id;
import java.util.Date;

@Data
@Entity
@AllArgsConstructor
@NoArgsConstructor
public class Machine {

    @Id
    private String macAddress;

    private String friendlyName;

    private Date lastAttestation;
}
