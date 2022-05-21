package ro.dissertation.dbapp.web.dto;

import lombok.Data;
import ro.dissertation.dbapp.model.IotRecord;

import java.util.Date;

@Data
public class IotRecordDto {

    private long id;

    private String datatype;

    private String value;

    private Date timestamp;

    public IotRecordDto(IotRecord record) {
        this.id = record.getId();
        this.datatype = record.getDatatype();
        this.value = record.getValue();
        this.timestamp = record.getTimestamp();
    }
}
