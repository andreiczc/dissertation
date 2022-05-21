package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.PagingAndSortingRepository;
import ro.dissertation.dbapp.model.IotObject;

public interface IotObjectRepository extends PagingAndSortingRepository<IotObject, Integer> {
}
