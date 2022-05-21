package ro.dissertation.dbapp.service.impl;

import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.repo.MachineRepository;
import ro.dissertation.dbapp.service.api.MachineService;
import ro.dissertation.dbapp.web.dto.MachineUpdateDto;

import java.util.List;
import java.util.NoSuchElementException;

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

    @Override
    public List<Machine> get(Pageable page) {
        return repository
                .findAll(page)
                .toList();
    }

    @Override
    public Machine update(MachineUpdateDto dto) {
        var optional = repository.findById(dto.getMacAddress());
        var item = optional.orElseThrow(NoSuchElementException::new);
        item.setFriendlyName(dto.getFriendlyName());

        return repository.save(item);
    }
}
