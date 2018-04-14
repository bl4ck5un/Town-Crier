      var accounts = Web3Service.eth.accounts;
      console.log(accounts);
      console.log(Web3Service.eth.getBalance(accounts[0]));

      function filterPayment(ev) {
        return ev.event == "Payment" && ev.args.beneficiary == $scope.accounts[0];
      }
      //historical records
      var his = contractInstance.allEvents({fromBlock: 0, toBlock: 'latest'});
      his.get(function(error, result) {
        if (!error) {
          console.log("history");
          console.log(result.filter(filterPayment));
        }
      })

      //watch for new records
      var event = contractInstance.Payment();
      event.watch(function(error, result) {
        if (!error) {
          console.log("new");
          console.log(result);
          console.log(result.event);
          console.log(result.args.beneficiary);
        }
      });

