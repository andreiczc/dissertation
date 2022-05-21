package ro.dissertation.dbapp.service.api;

import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.IotResource;
import ro.dissertation.dbapp.web.dto.IotResourceUpdateDto;

import java.util.List;
import java.util.Optional;

public interface IotResourceService {

    IotResource save(IotResource resource);

    List<IotResource> getByObject(IotObject object);

    Optional<IotResource> getById(int id);

    IotResourceUpdateDto update(IotResourceUpdateDto dto);
}
