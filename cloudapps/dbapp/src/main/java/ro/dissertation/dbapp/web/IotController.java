package ro.dissertation.dbapp.web;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import ro.dissertation.dbapp.service.api.IotService;
import ro.dissertation.dbapp.web.dto.MachineRequestDto;
import ro.dissertation.dbapp.web.dto.MachineResponseDto;

import javax.validation.Valid;

@RestController
@RequestMapping("/")
public class IotController {

    private final IotService service;

    public IotController(IotService service) {
        this.service = service;
    }

    @PostMapping("/upsert")
    public ResponseEntity<MachineResponseDto> upsert(@RequestBody @Valid MachineRequestDto requestDto) {
        return ResponseEntity
                .ok()
                .body(service.upsert(requestDto));
    }
}
