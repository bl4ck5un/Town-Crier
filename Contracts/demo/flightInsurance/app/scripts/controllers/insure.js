'use strict';

/**
 * @ngdoc function
 * @name flightInsuranceApp.controller:InsureCtrl
 * @description
 * # InsureCtrl
 * Controller of the flightInsuranceApp
 */
angular.module('flightInsuranceApp')
  .controller('InsureCtrl', function ($scope, Web3Service, $state, FI_ABI_ARRAY, FI_CONTRACT_ADDRESS) {
    $scope.accounts = [];
    $scope.flightData = {code: '', date: '', time: '', zone:''};
    $scope.policy = {amount: ''};
    $scope.selectedAccount = "";
    $scope.transactionHash = "";
    $scope.timezones = ["GMT", "GMT-1", "GMT-2", "GMT-3", "GMT-4", "GMT-5", "GMT-6", "GMT-7", "GMT-8", "GMT-9", "GMT-10", "GMT-11", "GMT-12",
                        "GMT+1", "GMT+2", "GMT+3", "GMT+4", "GMT+5", "GMT+6", "GMT+7", "GMT+8", "GMT+9", "GMT+10", "GMT+11", "GMT+12", "GMT+13", "GMT+14"];

    //$scope.connected = false;
  	//$scope.connected = Web3Service.isConnected();

    Web3Service.eth.getAccounts(function(error, result){
      if (error) {
        console.error("DOESNT COMPUTE ! ;)");
      }
      else {
        var i;
        for (i in result) {
          $scope.accounts.push({address: result[i], balance: Web3Service.eth.getBalance(result[i]) / Math.pow(10, 18)});
        }
        $scope.selectAccount($scope.accounts[0].address);
      }
    });

    $scope.selectAccount = function(account) {
      Web3Service.eth.defaultAccount = account.address;
      $scope.selectedAccount = account.address;
    }

    $scope.insure = function() {
      var MyContract = Web3Service.eth.contract(FI_ABI_ARRAY);
      var contractInstance = MyContract.at(FI_CONTRACT_ADDRESS);
      console.log($scope.policy.amount);
      var price = $scope.policy.amount * 1e+18 + (55 * 5e+13);
      console.log(price);
      var tmp = $scope.flightData.date.toString().substr(0, 16) + $scope.flightData.time.toString().substr(16, 9) + $scope.flightData.zone;
      console.log(tmp);
      var time = new Date(tmp);
      console.log(time.getTime());
      var result = contractInstance.insure([Web3Service.fromAscii($scope.flightData.code, 32), 
                        Web3Service.fromAscii((Math.floor(time.getTime() / 60) * 60).toString(), 32)], 
                        $scope.policy.amount, {value: price, gas: 3000000}, function (error, result){
        if (error) {
          console.error("RAAAAAH WHY DO NOTHING EVER WORKS !");
          console.error(error);
        }
        else {
          $scope.transactionHash = result;
          console.log(result);
          $state.go("insure.done")

        }
      });
    };

  });

