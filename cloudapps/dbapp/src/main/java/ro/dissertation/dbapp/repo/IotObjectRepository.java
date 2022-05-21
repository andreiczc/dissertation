package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.CrudRepository;
import ro.dissertation.dbapp.model.IotObject;

public interface IotObjectRepository extends CrudRepository<IotObject, Integer> {
}
