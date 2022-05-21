package ro.dissertation.dbapp.repo;

import org.springframework.data.repository.PagingAndSortingRepository;
import ro.dissertation.dbapp.model.IotRecord;

public interface IotRecordRepository extends PagingAndSortingRepository<IotRecord, Integer> {
}
