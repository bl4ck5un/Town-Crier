'use strict';

/**
 * @ngdoc overview
 * @name flightInsuranceApp
 * @description
 * # flightInsuranceApp
 *
 * Main module of the application.
 */
angular
  .module('flightInsuranceApp', [
    'ngAnimate',
    'ngCookies',
    'ngResource',
    'ngRoute',
    'ngSanitize',
    'ui.router',
    'ui.bootstrap'
  ])
  .config(function ($urlRouterProvider, $stateProvider) {
    $urlRouterProvider.otherwise("/main");

    $stateProvider
      .state('main', {
        templateUrl:  'views/main.html',
        controller:   'MainCtrl',
        url:          '/main'
      })

      .state('insure', {
        templateUrl:  'views/insure/layout.html',
        controller:   'InsureCtrl',
        url:          '/insure'
      })
      .state('insure.account', {
        url: '/account',
        templateUrl:  'views/insure/account.html',
      })
      .state('insure.flightData', {
        url: '/flightData',
        templateUrl:  'views/insure/flightData.html',
      })
      .state('insure.done', {
        url: '/done',
        templateUrl:  'views/insure/done.html',
      })
      .state('check', {
        templateUrl:  'views/check/layout.html',
        controller:   'CheckCtrl',
        url:          '/check'
      })
      .state('check.list', {
        templateUrl:  'views/check/list.html',
        url:          '/list'
      })
      .state('check.details', {
        templateUrl:  'views/check/details.html',
        url:          '/details'
      })
      .state('tc', {
        templateUrl:  'views/tc.html',
        controller:   'TCCtrl',
        url:          '/details'
      });
  })

