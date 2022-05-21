package ro.dissertation.dbapp.web;

import org.springframework.data.domain.PageRequest;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.service.api.IotService;
import ro.dissertation.dbapp.web.dto.IotObjectWithInstancesDto;
import ro.dissertation.dbapp.web.dto.IotResourceWithValuesDto;
import ro.dissertation.dbapp.web.dto.MachineRequestDto;
import ro.dissertation.dbapp.web.dto.MachineResponseDto;

import javax.validation.Valid;
import java.util.List;

@RestController
@RequestMapping("/object")
public class IotObjectController {

    private final IotService service;

    public IotObjectController(IotService service) {
        this.service = service;
    }

    @PostMapping("/upsert")
    public ResponseEntity<MachineResponseDto> upsert(@RequestBody @Valid MachineRequestDto requestDto) {
        return ResponseEntity
                .ok()
                .body(service.upsert(requestDto));
    }

    @GetMapping("/{objectId}")
    public ResponseEntity<List<IotResourceWithValuesDto>> getResourcesByObjectId(@PathVariable int objectId) {
        return ResponseEntity
                .ok()
                .body(service.getResources(objectId));
    }

    @GetMapping("/{objectId}/{resourceId}")
    public ResponseEntity<List<IotRecord>> getRecordsByResourceId(@PathVariable int objectId, @PathVariable int resourceId,
                                                                  @RequestParam(name = "pageSize", required = false, defaultValue = "50") int pageSize,
                                                                  @RequestParam(name = "pageNo", required = false, defaultValue = "0") int pageNo) {
        return ResponseEntity
                .ok()
                .body(service.getRecords(objectId, resourceId, PageRequest.of(pageNo, pageSize)));
    }

    @GetMapping("/")
    public ResponseEntity<Iterable<IotObjectWithInstancesDto>> getObjects(@RequestParam(name = "pageSize", required = false, defaultValue = "50") int pageSize,
                                                                          @RequestParam(name = "pageNo", required = false, defaultValue = "0") int pageNo) {
        return ResponseEntity
                .ok()
                .body(service.getObjects(PageRequest.of(pageNo, pageSize)));
    }

    @GetMapping("/machine")
    public ResponseEntity<Iterable<Machine>> getMachines(@RequestParam(name = "pageSize", required = false, defaultValue = "50") int pageSize,
                                                         @RequestParam(name = "pageNo", required = false, defaultValue = "0") int pageNo) {
        return ResponseEntity
                .ok()
                .body(service.getMachines(PageRequest.of(pageNo, pageSize)));
    }

    @PostMapping("/record")
    public ResponseEntity<IotRecord> saveRecord(@RequestBody IotRecord record) {
        return ResponseEntity
                .ok()
                .body(service.saveRecord(record));
    }
}
