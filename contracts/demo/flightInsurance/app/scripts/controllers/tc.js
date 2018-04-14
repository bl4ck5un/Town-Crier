'use strict';

/**
 * @ngdoc function
 * @name Flight Insurance App.controller:InsureCtrl
 * @description
 * # InsureCtrl
 * Controller of the Flight Insurance App
 */
angular.module('Flight Insurance App')
  .controller('TCCtrl', function ($scope, Web3Service, $state, TC_ABI_ARRAY, TC_CONTRACT_ADDRESS, FI_ABI_ARRAY, FI_CONTRACT_ADDRESS) {

    Web3Service.eth.getAccounts(function(error, result){
      if (error) {
        console.error("DOESNT COMPUTE ! ;)");
      }
      else {
          Web3Service.eth.defaultAccount = result[0];
      }
    });

    $scope.deliver = function() {
      /*var filterOne = function (ev) {
        return ev.event == "PolicyStatus" && ev.args.policyid == $scope.policyDelay.code;
      };
      var MyContract = Web3Service.eth.contract(FI_ABI_ARRAY);
      var contractInstance = MyContract.at(FI_CONTRACT_ADDRESS);
      var his = contractInstance.allEvents({fromBlock: 0, toBlock: 'latest'});
      var po = []

      his.get(function(error, result) {
        if (!error) {
          po = result.filter(filterOne);
          console.log(result);
          console.log(po[0]);
          Web3Service.eth.defaultAccount = po[0].args.beneficiary;
        }
      })
*/

      var MyContract = Web3Service.eth.contract(TC_ABI_ARRAY);
      var contractInstance = MyContract.at(TC_CONTRACT_ADDRESS);

      console.log(contractInstance);
      
/*      his = contractInstance.allEvents({fromBlock: 0, toBlock: 'latest'});

      var filterAll = function (ev) {
        return ev.event == "PolicyStatus" && ev.args.beneficiary == $scope.selectedAccount;
      };


      his.get(function(error, result) {
        if (!error) {
          $scope.allPolicies = result.filter(filterAll);
          console.log($scope.allPolicies);
          console.log(result);
        }
      })*/console.log(Web3Service.toHex($scope.policyDelay.delay));

      var result = contractInstance.deliver($scope.policyDelay.code,  "0x" + ("0000000000000000000000000000000000000000000000000000000000000000" + new Number($scope.policyDelay.delay).toString(16)).slice(-64), {value: 0, gas: 200000}, function (error, result){
        if (error) {
          console.error("RAAAAAH WHY DO NOTHING EVER WORKS !");
          console.error(error);
        }
        else {
          $scope.transactionHash = result;
          console.log(result);
          $scope.$apply(function () {
            $scope.policyDelay.code = '';
            $scope.policyDelay.delay = '';
          });

        }
      });
    };


  });

