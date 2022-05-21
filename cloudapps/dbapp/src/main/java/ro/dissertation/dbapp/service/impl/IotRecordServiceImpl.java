package ro.dissertation.dbapp.service.impl;

import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.model.IotResource;
import ro.dissertation.dbapp.repo.IotRecordRepository;
import ro.dissertation.dbapp.service.api.IotRecordService;

import java.util.List;

@Service
public class IotRecordServiceImpl implements IotRecordService {

    private final IotRecordRepository repository;

    public IotRecordServiceImpl(IotRecordRepository repository) {
        this.repository = repository;
    }

    @Override
    public List<IotRecord> getByResource(IotResource resource, Pageable page) {
        return repository.findAllByResourceOrderByTimestampDesc(resource, page);
    }

    @Override
    public IotRecord save(IotRecord record) {
        return repository.save(record);
    }
}
