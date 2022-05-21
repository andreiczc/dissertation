package ro.dissertation.dbapp.repo;

import org.springframework.data.domain.Pageable;
import org.springframework.data.repository.PagingAndSortingRepository;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.model.IotResource;

import java.util.List;

public interface IotRecordRepository extends PagingAndSortingRepository<IotRecord, Integer> {
    List<IotRecord> findAllByResourceOrderByTimestampDesc(IotResource resource, Pageable pageable);
}
