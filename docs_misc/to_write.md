# Things to write about in the docs

- configuring mosquitto

if using certificates -> they can be extracted and the node impersonated
if using some sort of challenge-response, specify the mac address in an extension ?!

what can not be considered secure:

- private key to be securely stored at all times
- if private key is compromised, so is the MAC of the chip

## What needs to be stored in FS

- html data
- ca certificate with which the server cert has been signed
