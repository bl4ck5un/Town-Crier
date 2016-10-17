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
    $scope.flightData = {code: '', date: '', time: ''};
    $scope.policy = {amount: ''};
    $scope.selectedAccount = "";
    $scope.transactionHash = "";

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
      var result = contractInstance.insure([Web3Service.fromAscii($scope.flightData.code, 32)], $scope.policy.amount, {value: price, gas: 3000000}, function (error, result){
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

