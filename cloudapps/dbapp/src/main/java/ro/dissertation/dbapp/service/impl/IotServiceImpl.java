package ro.dissertation.dbapp.service.impl;

import org.springframework.data.domain.PageRequest;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import ro.dissertation.dbapp.model.*;
import ro.dissertation.dbapp.service.api.*;
import ro.dissertation.dbapp.web.dto.*;

import org.springframework.data.domain.Pageable;

import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.stream.StreamSupport;

@Service
public class IotServiceImpl implements IotService {

    private final MachineService machineService;
    private final IotObjectService objectService;
    private final IotInstanceService instanceService;
    private final IotResourceService resourceService;
    private final IotRecordService recordService;

    public IotServiceImpl(MachineService machineService, IotObjectService objectService, IotInstanceService instanceService, IotResourceService resourceService, IotRecordService recordService) {
        this.machineService = machineService;
        this.objectService = objectService;
        this.instanceService = instanceService;
        this.resourceService = resourceService;
        this.recordService = recordService;
    }

    @Transactional
    @Override
    public MachineResponseDto upsert(MachineRequestDto machine) {
        var response = new MachineResponseDto();
        var objectList = new ArrayList<IotObjectResponseDto>();
        response.setObjects(objectList);

        var machineItemTemp = new Machine(machine.getMacAddress(), "", machine.getLastAttestation());
        var machineItem = machineService.upsert(machineItemTemp);
        response.setMacAddress(machine.getMacAddress());

        machine.getObjects().forEach(iotObjectDto -> {
            var iotObjectTemp = new IotObject();
            iotObjectTemp.setObjectId(iotObjectDto.getObjectId());

            var iotObject = objectService.upsert(iotObjectTemp);

            var instance = new IotInstance();
            instance.setMachine(machineItem);
            instance.setObject(iotObject);

            instance = instanceService.save(instance);

            var responseDto = new IotObjectResponseDto();
            responseDto.setObjectId(iotObject.getObjectId());
            responseDto.setInstanceId(instance.getInstanceId());
            objectList.add(responseDto);

            iotObjectDto.getResources().forEach(iotResourceDto -> {
                var iotResource = new IotResource();
                iotResource.setResourceId(iotResourceDto.getResourceId());
                iotResource.setObject(iotObject);

                resourceService.save(iotResource);
            });
        });


        return response;
    }

    @Override
    public List<IotResourceWithValuesDto> getResources(int objectId) {
        var object = new IotObject();
        object.setObjectId(objectId);

        var resources = resourceService.getByObject(object);

        return resources
                .stream()
                .map(iotResource -> {
                    var dto = new IotResourceWithValuesDto();
                    var list = new ArrayList<IotRecordDto>();
                    dto.setResourceId(iotResource.getResourceId());
                    dto.setFriendlyName(iotResource.getFriendlyName());
                    dto.setRecords(list);
                    var records = recordService
                            .getByResource(iotResource, PageRequest.of(0, 50));

                    records.forEach(record -> {
                        var curr = new IotRecordDto(record);
                        list.add(curr);
                    });

                    return dto;
                })
                .toList();
    }

    @Override
    public Iterable<IotObjectWithInstancesDto> getObjects(Pageable page) {
        var allObjects = objectService.getAll(page);

        return StreamSupport
                .stream(allObjects.spliterator(), false)
                .map(object -> {
                    var dto = new IotObjectWithInstancesDto();
                    dto.setObjectId(object.getObjectId());

                    var instances = instanceService.getByObject(object);
                    var list = new ArrayList<IotInstanceWithMacDto>();
                    instances.forEach(instance -> {
                        var curr = new IotInstanceWithMacDto();
                        curr.setInstanceId(instance.getInstanceId());
                        curr.setMacAddress(instance.getMachine().getMacAddress());

                        list.add(curr);
                    });

                    dto.setInstances(list);

                    return dto;
                }).toList();
    }

    @Override
    public Iterable<Machine> getMachines(Pageable page) {
        return machineService.get(page);
    }

    @Override
    public IotRecord saveRecord(IotRecord record) {
        return recordService.save(record);
    }

    @Override
    public List<IotRecordDto> getRecords(int objectId, int resourceId, Pageable page) {
        var optional = resourceService.getById(resourceId);
        var resource = optional.orElseThrow(NoSuchElementException::new);
        if (resource.getObject().getObjectId() != objectId) {
            throw new RuntimeException();
        }

        return recordService
                .getByResource(resource, page)
                .stream()
                .map(IotRecordDto::new)
                .toList();
    }

    @Override
    public Machine updateMachine(MachineUpdateDto dto) {
        return machineService.update(dto);
    }

    @Override
    public IotObjectUpdateDto updateObject(IotObjectUpdateDto dto) {
        return objectService.update(dto);
    }

    @Override
    public IotResourceUpdateDto editResource(IotResourceUpdateDto dto) {
        return resourceService.update(dto);
    }
}
