package ro.dissertation.dbapp.service.impl;

import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.repo.IotObjectRepository;
import ro.dissertation.dbapp.service.api.IotObjectService;

import org.springframework.data.domain.Pageable;
import ro.dissertation.dbapp.web.dto.IotObjectUpdateDto;

import java.util.NoSuchElementException;


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

    @Override
    public Iterable<IotObject> getAll(Pageable page) {
        return repository.findAll(page);
    }

    @Override
    public IotObjectUpdateDto update(IotObjectUpdateDto dto) {
        var optional = repository.findById(dto.getObjectId());
        var item = optional.orElseThrow(NoSuchElementException::new);
        item.setFriendlyName(dto.getFriendlyName());
        item = repository.save(item);

        return dto;
    }
}
