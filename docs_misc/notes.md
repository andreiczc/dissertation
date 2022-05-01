# Notes

## Ideas for tinyML

- predict earthquakes based on earth movement -> maybe RNN, more investigation necessary
- using a CNN to track employees coming and leaving (LeNet5 in tf lite converted to tinyML model)

## Securing PSK

- deleting the psk associated with the client doesn't affect the connection
- as soon as the client is connected, delete that and CHECK THE MEMORY

## Things to remember

0x04 is Uncompressed point indicator!
always open port both on firewall and ingress rule

## TODO

- add certificate read from SPIFFS
- understand ethereum smart contracts
- make arduino lib compile under esp-idf

## Ideas

- node with multiple "sensor configurations"
- comes barebones and sensors can be added a later date
- management interface that allows controller of which sensor data to report
- allows blockchain storage for fields which are considered very sensitive -> input the address of the smart contract
- access control -> only the owner can add new users
- allow pluggable ML at the level of data ??? -> is it possible?
- link: (<https://medium.com/@dmytro.korablyov/first-steps-with-esp32-and-tensorflow-lite-for-microcontrollers-c2d8e238accf>)
- map the model with names in memory
- allow storage of only up to 5 models at a time
- when querying for data, check if the data is marked as ML and adjust payload as needed
- if actuators were used, allow custom commands for that... but no actuators are actually used in this thesis
