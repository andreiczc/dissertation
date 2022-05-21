package ro.dissertation.dbapp.service.api;

import ro.dissertation.dbapp.model.Machine;

public interface MachineService {

    Machine upsert(Machine machine);
}
