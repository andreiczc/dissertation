package ro.dissertation.dbapp.service.impl;

import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.IotInstance;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.repo.IotInstanceRepository;
import ro.dissertation.dbapp.service.api.IotInstanceService;

import java.util.Optional;

@Service
public class IotInstanceServiceImpl implements IotInstanceService {

    private final IotInstanceRepository repository;

    public IotInstanceServiceImpl(IotInstanceRepository repository) {
        this.repository = repository;
    }

    @Override
    public IotInstance save(IotInstance instance) {
        var dbOptional = this.get(instance.getObject(), instance.getMachine());
        if (dbOptional.isPresent()) {
            return dbOptional.get();
        }

        var optional = repository.findFirstByObjectOrderByInstanceIdDesc(instance.getObject());
        if (optional.isEmpty()) {
            instance.setInstanceId(0);
            repository.save(instance);
        } else {
            var item = optional.get();
            instance.setInstanceId(item.getInstanceId() + 1);
            repository.save(instance);
        }

        return instance;
    }

    @Override
    public Optional<IotInstance> get(IotObject object, Machine machine) {
        return repository.findFirstByObjectAndMachine(object, machine);
    }

    @Override
    public Iterable<IotInstance> getByObject(IotObject object) {
        return repository.findByObject(object);
    }
}
