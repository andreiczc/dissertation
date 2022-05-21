package ro.dissertation.dbapp.service.api;

import org.springframework.data.domain.Pageable;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.web.dto.IotObjectWithInstancesDto;
import ro.dissertation.dbapp.web.dto.IotResourceWithValuesDto;
import ro.dissertation.dbapp.web.dto.MachineRequestDto;
import ro.dissertation.dbapp.web.dto.MachineResponseDto;

import java.util.List;

public interface IotService {

    MachineResponseDto upsert(MachineRequestDto machine);

    List<IotResourceWithValuesDto> getResources(int objectId);

    Iterable<IotObjectWithInstancesDto> getObjects(Pageable page);

    Iterable<Machine> getMachines(Pageable page);

    IotRecord saveRecord(IotRecord record);

    List<IotRecord> getRecords(int objectId, int resourceId, Pageable page);
}
