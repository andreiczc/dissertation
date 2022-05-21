package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.CrudRepository;
import ro.dissertation.dbapp.model.IotInstance;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.Machine;

import java.util.Optional;

public interface IotInstanceRepository extends CrudRepository<IotInstance, Long> {

    Optional<IotInstance> findFirstByObjectOrderByInstanceIdDesc(IotObject object);

    Optional<IotInstance> findFirstByObjectAndMachine(IotObject object, Machine machine);
}
