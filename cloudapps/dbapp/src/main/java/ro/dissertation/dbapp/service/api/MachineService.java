package ro.dissertation.dbapp.service.api;

import org.springframework.data.domain.Pageable;
import ro.dissertation.dbapp.model.Machine;

import java.util.List;

public interface MachineService {

    Machine upsert(Machine machine);

    List<Machine> get(Pageable page);
}
