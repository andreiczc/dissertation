// SPDX-License-Identifier: MIT
pragma solidity ^0.8.7;

contract SensitiveStorage {
    struct Entry {
        uint256 timestamp;
        string sensorType;
        uint256 value;
        string origin;
    }

    Entry[] private entries;
    address private owner;
    mapping(address => string) private allowedUsers;
    mapping(string => bool) private sensorTypes;

    constructor() {
        owner = msg.sender;
        allowedUsers[msg.sender] = "owner";
    }

    function allowed(address addr) private view returns (bool) {
        return (bytes(allowedUsers[addr])).length != 0;
    }

    function sensorTypeSupported(string memory kind)
        private
        view
        returns (bool)
    {
        return sensorTypes[kind];
    }

    function getEntries() public view returns (Entry[] memory) {
        if (!allowed(msg.sender)) {
            revert("Not authorized!");
        }

        return entries;
    }

    function insertEntry(string memory datatype, uint256 value) public {
        if (!allowed(msg.sender) || !sensorTypeSupported(datatype)) {
            revert("Not authorized!");
        }

        entries.push(
            Entry(block.timestamp, datatype, value, allowedUsers[msg.sender])
        );
    }

    function allowUser(address addr, string memory nickname) public {
        if (msg.sender != owner) {
            revert("Not authorized!");
        }

        allowedUsers[addr] = nickname;
    }

    function removeUser(address addr) public {
        if (msg.sender != owner) {
            revert("Not authorized!");
        }

        allowedUsers[addr] = "";
    }

    function addSensorType(string memory kind) public {
        if (msg.sender != owner) {
            revert("Not authorized!");
        }

        sensorTypes[kind] = true;
    }

    function removeSensorType(string memory kind) public {
        if (msg.sender != owner) {
            revert("Not authorized!");
        }

        sensorTypes[kind] = false;
    }
}
