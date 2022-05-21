package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.CrudRepository;
import ro.dissertation.dbapp.model.IotResource;

public interface IotResourceRepository extends CrudRepository<IotResource, Integer> {
}
