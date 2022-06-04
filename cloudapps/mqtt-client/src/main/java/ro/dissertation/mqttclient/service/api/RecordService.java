package ro.dissertation.mqttclient.service.api;

import ro.dissertation.mqttclient.model.IotRecord;
import ro.dissertation.mqttclient.model.IpsoValue;

import java.util.Date;

public interface RecordService {

    IotRecord persistRecord(IpsoValue object, Date timestamp);
}
