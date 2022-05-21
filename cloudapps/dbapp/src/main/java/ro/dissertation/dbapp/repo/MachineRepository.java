package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.PagingAndSortingRepository;
import ro.dissertation.dbapp.model.Machine;

public interface MachineRepository extends PagingAndSortingRepository<Machine, String> {
}
