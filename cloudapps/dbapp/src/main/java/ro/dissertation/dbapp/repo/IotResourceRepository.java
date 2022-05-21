package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.CrudRepository;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.IotResource;

import java.util.List;

public interface IotResourceRepository extends CrudRepository<IotResource, Integer> {
    List<IotResource> findByObject(IotObject object);
}
