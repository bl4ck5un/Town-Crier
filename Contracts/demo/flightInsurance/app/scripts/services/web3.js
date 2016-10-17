'use strict';

angular.module('flightInsuranceApp')
.factory('Web3Service', function($http, $q, $window){
  //var web3
  // Init a new web3 object, unless we're in Mist
  if (typeof web3 !== 'undefined') {
    web3 = new Web3(web3.currentProvider);
  } else {
    // set the provider you want from Web3.providers
    web3 = new Web3(new Web3.providers.HttpProvider("http://localhost:8102"));
  }

  return web3;
});
