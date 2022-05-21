package ro.dissertation.dbapp.service.api;

import org.springframework.data.domain.Pageable;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.model.IotResource;

import java.util.List;

public interface IotRecordService {

    List<IotRecord> getByResource(IotResource resource, Pageable page);

    IotRecord save(IotRecord record);
}
