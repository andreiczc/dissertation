package ro.dissertation.dbapp.web.dto;

import lombok.Data;
import ro.dissertation.dbapp.model.IotResource;

@Data
public class IotResourceDto {

    private int resourceId;
    private String friendlyName;

    public IotResourceDto(IotResource resource) {
        this.resourceId = resource.getResourceId();
        this.friendlyName = resource.getFriendlyName();
    }
}
