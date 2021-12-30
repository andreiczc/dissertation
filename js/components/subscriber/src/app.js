const LOG = require("log4js").getLogger();
const cbor = require("cbor");

LOG.level = "debug";

LOG.info("Application starting...");

const message =
  "a26974696d657374616d70016676616c75657381a5686f626a6563744964fa3f8000006a696e7374616e63654964fa400000006a7265736f757263654964fa4040000068646174617479706565466c6f61746576616c7565fb4008f5c28f5c28f6";

const decodedMessage = cbor.decodeAllSync(message);
LOG.info("Result is: ", JSON.stringify(decodedMessage));
