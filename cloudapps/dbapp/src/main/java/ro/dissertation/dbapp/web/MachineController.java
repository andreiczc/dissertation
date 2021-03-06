package ro.dissertation.dbapp.web;

import org.springframework.data.domain.PageRequest;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.service.api.IotService;
import ro.dissertation.dbapp.web.dto.MachineRequestDto;
import ro.dissertation.dbapp.web.dto.MachineResponseDto;
import ro.dissertation.dbapp.web.dto.MachineUpdateDto;

import javax.validation.Valid;

@RestController
@RequestMapping("/")
public class MachineController {

    private final IotService service;

    public MachineController(IotService service) {
        this.service = service;
    }

    @PostMapping("/upsert")
    public ResponseEntity<MachineResponseDto> upsert(@RequestBody @Valid MachineRequestDto requestDto) {
        return ResponseEntity
                .ok()
                .body(service.upsert(requestDto));
    }

    @GetMapping("/machine")
    public ResponseEntity<Iterable<Machine>> getMachines(@RequestParam(name = "pageSize", required = false, defaultValue = "50") int pageSize,
                                                         @RequestParam(name = "pageNo", required = false, defaultValue = "0") int pageNo) {
        return ResponseEntity
                .ok()
                .body(service.getMachines(PageRequest.of(pageNo, pageSize)));
    }

    @PutMapping("/machine/edit")
    public ResponseEntity<Machine> updateMachine(@RequestBody @Valid MachineUpdateDto dto) {
        return ResponseEntity
                .ok()
                .body(service.updateMachine(dto));
    }
}
