package ro.dissertation.dbapp.service.api;

import ro.dissertation.dbapp.model.IotInstance;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.Machine;

import java.util.Optional;

public interface IotInstanceService {

    IotInstance save(IotInstance instance);

    Optional<IotInstance> get(IotObject object, Machine machine);

    Iterable<IotInstance> getByObject(IotObject object);
}
