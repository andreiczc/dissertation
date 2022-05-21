package ro.dissertation.dbapp.service.impl;

import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import ro.dissertation.dbapp.model.IotInstance;
import ro.dissertation.dbapp.model.IotObject;
import ro.dissertation.dbapp.model.IotResource;
import ro.dissertation.dbapp.model.Machine;
import ro.dissertation.dbapp.service.api.*;
import ro.dissertation.dbapp.web.dto.IotObjectResponseDto;
import ro.dissertation.dbapp.web.dto.MachineRequestDto;
import ro.dissertation.dbapp.web.dto.MachineResponseDto;

import java.util.ArrayList;

@Service
public class IotServiceImpl implements IotService {

    private final MachineService machineService;
    private final IotObjectService objectService;
    private final IotInstanceService instanceService;
    private final IotResourceService resourceService;

    public IotServiceImpl(MachineService machineService, IotObjectService objectService, IotInstanceService instanceService, IotResourceService resourceService) {
        this.machineService = machineService;
        this.objectService = objectService;
        this.instanceService = instanceService;
        this.resourceService = resourceService;
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
}
