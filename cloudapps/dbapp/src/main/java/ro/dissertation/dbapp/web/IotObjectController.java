package ro.dissertation.dbapp.web;

import org.springframework.data.domain.PageRequest;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import ro.dissertation.dbapp.service.api.IotService;
import ro.dissertation.dbapp.web.dto.IotObjectWithInstancesDto;
import ro.dissertation.dbapp.web.dto.IotResourceWithValuesDto;

import java.util.List;

@RestController
@RequestMapping("/")
public class IotObjectController {

    private final IotService service;

    public IotObjectController(IotService service) {
        this.service = service;
    }

    @GetMapping("/{objectId}")
    public ResponseEntity<List<IotResourceWithValuesDto>> getResourcesByObjectId(@PathVariable int objectId) {
        return ResponseEntity
                .ok()
                .body(service.getResources(objectId));
    }

    @GetMapping("/")
    public ResponseEntity<Iterable<IotObjectWithInstancesDto>> getObjects(@RequestParam(name = "pageSize", required = false, defaultValue = "50") int pageSize,
                                                                          @RequestParam(name = "pageNo", required = false, defaultValue = "0") int pageNo) {
        return ResponseEntity
                .ok()
                .body(service.getObjects(PageRequest.of(pageNo, pageSize)));
    }
}
