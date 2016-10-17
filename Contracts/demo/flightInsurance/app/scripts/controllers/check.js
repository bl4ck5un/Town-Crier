'use strict';

/**
 * @ngdoc function
 * @name flightInsuranceApp.controller:CheckCtrl
 * @description
 * # CheckCtrl
 * Controller of the flightInsuranceApp
 */
angular.module('flightInsuranceApp')
  .controller('CheckCtrl', function ($scope, Web3Service, $state, FI_ABI_ARRAY, FI_CONTRACT_ADDRESS) {
    $scope.accounts = [];
    $scope.flightData = {code: '', date: '', time: ''};
    $scope.selectedAccount = "";
    $scope.allPolicies = [];
    $scope.policyDetails = [];
    $scope.selectedPolicy = 0;

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
      console.log("Selected account : ", account);
      $scope.getAllPolicies();
    }



    $scope.getAllPolicies = function() {
      var filterAll = function (ev) {
        return ev.event == "Insure" && ev.args.beneficiary == $scope.selectedAccount;
      };
      var MyContract = Web3Service.eth.contract(FI_ABI_ARRAY);
      var contractInstance = MyContract.at(FI_CONTRACT_ADDRESS);
      var his = contractInstance.allEvents({fromBlock: 0, toBlock: 'latest'});
      his.get(function(error, result) {
        if (!error) {
          $scope.$apply(function () {
            $scope.allPolicies = result.filter(filterAll);
          });
          console.log("account:", $scope.selectedAccount);
          console.log("history");
          console.log($scope.allPolicies);
          console.log(result);
        }
      })
    }

    $scope.showPolicyDetails = function(policyid){
      $scope.selectedPolicy = policyid;
      console.log("SELECTED POLICY : " + $scope.selectedPolicy);
      var filterOne = function (ev) {
        return ev.event == "PolicyStatus" && ev.args.policyid.eq($scope.selectedPolicy);
      };
      var MyContract = Web3Service.eth.contract(FI_ABI_ARRAY);
      var contractInstance = MyContract.at(FI_CONTRACT_ADDRESS);
      var his = contractInstance.allEvents({fromBlock: 0, toBlock: 'latest'});
      his.get(function(error, result) {
        if (!error) {
          $scope.$apply(function () {
            $scope.policyDetails = result.filter(filterOne);
          });
          console.log("history");
          console.log($scope.policyDetails);
          $state.go("check.details");
        }
      })
    }
  });

