cat <<EOF > compiledContract.js
var compiledContract = $(cat ../TownCrier.sol ../Application.sol ../FlightInsurance.sol | solc  --optimize --combined-json abi,bin,interface  | sed 's/<stdin>\://g');
EOF
