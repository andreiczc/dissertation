package ro.dissertation.dbapp.service.api;

import ro.dissertation.dbapp.model.IotObject;

public interface IotObjectService {

    IotObject upsert(IotObject object);
}
