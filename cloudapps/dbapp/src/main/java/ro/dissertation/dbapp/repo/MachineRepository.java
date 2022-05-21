package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.CrudRepository;
import ro.dissertation.dbapp.model.Machine;

public interface MachineRepository extends CrudRepository<Machine, String> {
}
