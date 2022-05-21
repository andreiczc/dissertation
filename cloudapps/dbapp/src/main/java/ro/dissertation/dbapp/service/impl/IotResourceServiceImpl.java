package ro.dissertation.dbapp.service.impl;

import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.IotResource;
import ro.dissertation.dbapp.repo.IotResourceRepository;
import ro.dissertation.dbapp.service.api.IotResourceService;
import ro.dissertation.dbapp.web.dto.IotResourceUpdateDto;

import java.util.List;
import java.util.NoSuchElementException;
import java.util.Optional;

@Service
public class IotResourceServiceImpl implements IotResourceService {

    private final IotResourceRepository repository;

    public IotResourceServiceImpl(IotResourceRepository repository) {
        this.repository = repository;
    }

    @Override
    public IotResource save(IotResource resource) {
        var dbOptional = repository.findById(resource.getResourceId());
        var item = dbOptional.orElse(resource);

        return repository.save(item);
    }

    @Override
    public List<IotResource> getByObject(IotObject object) {
        return repository
                .findByObject(object);
    }

    @Override
    public Optional<IotResource> getById(int id) {
        return repository.findById(id);
    }

    @Override
    public IotResourceUpdateDto update(IotResourceUpdateDto dto) {
        var optional = repository.findById(dto.getResourceId());
        var item = optional.orElseThrow(NoSuchElementException::new);
        item.setFriendlyName(dto.getFriendlyName());
        item = repository.save(item);

        return dto;
    }
}
