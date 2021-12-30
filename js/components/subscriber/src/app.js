const LOG = require("log4js").getLogger();
const cbor = require("cbor");

LOG.level = "debug";

LOG.info("Application starting...");

const testPayload =
  "a2 69 74 69 6d 65 73 74 61 6d 70 01 66 76 61 6c 75 65 73 81 a5 68 6f 62 6a 65 63 74 49 64 fa 3f 80 00 00 6a 69 6e 73 74 61 6e 63 65 49 64 fa 40 00 00 00 6a 72 65 73 6f 75 72 63 65 49 64 fa 40 40 00 00 68 64 61 74 61 74 79 70 65 65 46 6c 6f 61 74 65 76 61 6c 75 65 fb 40 08 f5 c2 8f 5c 28 f6";

const decodedMessage = cbor.decodeAllSync(testPayload);
LOG.info("Result is: ", decodedMessage);
