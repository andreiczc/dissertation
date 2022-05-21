package ro.dissertation.dbapp.service.impl;

import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.repo.IotObjectRepository;
import ro.dissertation.dbapp.service.api.IotObjectService;

@Service
public class IotObjectServiceImpl implements IotObjectService {

    private final IotObjectRepository repository;

    public IotObjectServiceImpl(IotObjectRepository repository) {
        this.repository = repository;
    }

    @Override
    public IotObject upsert(IotObject object) {
        var objectOptional = repository.findById(object.getObjectId());
        if (objectOptional.isEmpty()) {
            repository.save(object);
        }

        return object;
    }
}
