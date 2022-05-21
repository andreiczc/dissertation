package ro.dissertation.dbapp.service.api;

import ro.dissertation.dbapp.model.IotObject;

import org.springframework.data.domain.Pageable;

public interface IotObjectService {

    IotObject upsert(IotObject object);

    Iterable<IotObject> getAll(Pageable page);
}
