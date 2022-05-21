package ro.dissertation.dbapp.service.api;

import ro.dissertation.dbapp.web.dto.MachineRequestDto;
import ro.dissertation.dbapp.web.dto.MachineResponseDto;

public interface IotService {

    MachineResponseDto upsert(MachineRequestDto machine);
}
