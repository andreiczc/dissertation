package ro.dissertation.dbapp.service.impl;

import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.repo.MachineRepository;
import ro.dissertation.dbapp.service.api.MachineService;

@Service
public class MachineServiceImpl implements MachineService {

    private final MachineRepository repository;

    public MachineServiceImpl(MachineRepository repository) {
        this.repository = repository;
    }

    @Override
    public Machine upsert(Machine machine) {
        var machineOptional = repository.findById(machine.getMacAddress());

        var dbItem = machineOptional.orElse(machine);
        dbItem.setLastAttestation(machine.getLastAttestation());
        dbItem = repository.save(dbItem);

        return dbItem;
    }
}
