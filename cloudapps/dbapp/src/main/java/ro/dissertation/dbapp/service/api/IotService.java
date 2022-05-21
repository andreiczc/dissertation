package ro.dissertation.dbapp.service.api;

import org.springframework.data.domain.Pageable;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.web.dto.*;

import java.util.List;

public interface IotService {

    MachineResponseDto upsert(MachineRequestDto machine);

    List<IotResourceWithValuesDto> getResources(int objectId);

    Iterable<IotObjectWithInstancesDto> getObjects(Pageable page);

    Iterable<Machine> getMachines(Pageable page);

    IotRecord saveRecord(IotRecord record);

    List<IotRecordDto> getRecords(int objectId, int resourceId, Pageable page);

    Machine updateMachine(MachineUpdateDto dto);

    IotObjectUpdateDto updateObject(IotObjectUpdateDto dto);

    IotResourceUpdateDto editResource(IotResourceUpdateDto dto);
}
