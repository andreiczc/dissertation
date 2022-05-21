package ro.dissertation.dbapp.web;

import org.springframework.data.domain.PageRequest;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import ro.dissertation.dbapp.model.IotRecord;
import ro.dissertation.dbapp.service.api.IotService;
import ro.dissertation.dbapp.web.dto.IotRecordDto;

import java.util.List;

@RestController
@RequestMapping("/")
public class IotRecordController {

    private final IotService service;

    public IotRecordController(IotService service) {
        this.service = service;
    }

    @GetMapping("/{objectId}/{resourceId}")
    public ResponseEntity<List<IotRecordDto>> getRecordsByResourceId(@PathVariable int objectId, @PathVariable int resourceId,
                                                                     @RequestParam(name = "pageSize", required = false, defaultValue = "50") int pageSize,
                                                                     @RequestParam(name = "pageNo", required = false, defaultValue = "0") int pageNo) {
        return ResponseEntity
                .ok()
                .body(service.getRecords(objectId, resourceId, PageRequest.of(pageNo, pageSize)));
    }

    @PostMapping("/record")
    public ResponseEntity<IotRecord> saveRecord(@RequestBody IotRecord record) {
        return ResponseEntity
                .ok()
                .body(service.saveRecord(record));
    }
}
