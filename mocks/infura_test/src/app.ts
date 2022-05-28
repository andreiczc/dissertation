import Web3 from "web3";
import { TransactionConfig } from "web3-core";

async function main() {
  const baseUrl = "https://ropsten.infura.io/v3";
  const projectId = "8297ac153d3948b78c03d2aff759abef";
  const secret = "94f4c7a9705d44f483e71fe97a27dfe3";
  const ownAddress = "0xea8964911E02471f519300eE6D1eEc11236E3165";
  const contractAddress = "0x14cAfcCc3c0857A9281A22E4b74E770A99c29cB1";
  const privateKey =
    "afe65d226f1d8f3706cdf02d509166e43ccd17c8a08aba9d486b4f10c1192960";

  /* const headers = {
    "Content-Type": "application-json",
  };

  const payload = {
    jsonrpc: "2.0",
    method: "eth_getTransactionCount",
    params: [ownAddress, "latest"],
    id: 1,
  };

  const response = await axios({
    url: `${baseUrl}/${projectId}`,
    method: "POST",
    headers: headers,
    data: payload,
    auth: {
      username: "",
      password: secret,
    },
  });
  console.log(`Response has been received. Code: ${response.status}`);
  console.log(JSON.stringify(response.data)); */

  const web3 = new Web3(
    new Web3.providers.HttpProvider(`${baseUrl}/${projectId}`)
  );
  const signer = web3.eth.accounts.privateKeyToAccount(privateKey);
  web3.eth.accounts.wallet.add(signer);
  const types = ["string", "uint256"];
  const values = [
    "68756D6964697479",
    "0000000000000000000000000000000000000000000000000000000000000001",
  ];
  const methodSignature = "0x4b2c190d";
  const encodedFunctionCall = web3.eth.abi.encodeFunctionCall(
    {
      name: "insertEntry",
      type: "function",
      inputs: [
        { type: "string", name: "datatype" },
        { type: "uint256", name: "value" },
      ],
    },
    ["humidity", "56"]
  );

  const txParams: TransactionConfig = {
    from: signer.address,
    to: contractAddress,
    value: web3.utils.numberToHex(0),
    data: encodedFunctionCall,
    nonce: await web3.eth.getTransactionCount(signer.address),
    maxPriorityFeePerGas: web3.utils.toHex(web3.utils.toWei("2", "gwei")),
    chainId: 3,
  };

  const gas = await web3.eth.estimateGas(txParams);
  txParams.gas = gas;

  const signedTx = await web3.eth.accounts.signTransaction(
    txParams,
    signer.privateKey
  );
  const receipt = await web3.eth
    .sendSignedTransaction(signedTx.rawTransaction as string)
    .once("transactionHash", (txHash) => {
      console.log(`Mining transaction ...`);
      console.log(`https://ropsten.etherscan.io/tx/${txHash}`);
    });

  console.log(`Mined in block ${receipt.blockNumber}`);
}

main()
  .then(() => console.log("Done!"))
  .catch((error: Error) => console.log(`error encountered: ${error.message}`));
