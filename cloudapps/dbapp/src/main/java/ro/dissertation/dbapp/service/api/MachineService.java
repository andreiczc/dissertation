package ro.dissertation.dbapp.service.api;

import org.springframework.data.domain.Pageable;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.web.dto.MachineUpdateDto;

import java.util.List;

public interface MachineService {

    Machine upsert(Machine machine);

    List<Machine> get(Pageable page);

    Machine update(MachineUpdateDto dto);
}
