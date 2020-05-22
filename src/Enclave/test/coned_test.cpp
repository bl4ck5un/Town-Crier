//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TownCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include "../log.h"

#include "tls_client.h"
#include "scrapers/coned.h"
#include "pedersen_commit.h"

const char * websitedata = R"(
             <form action=\"\" class=\"account-details__info-container js-personal-details-form js-module\" data-module=\"AccountDetailsPersonal\" data-service-url=\"/sitecore/api/ssc/ConEd-Cms-Services-Controllers-Dcx/Account/0/UpdateAccount\" method=\"post\">        <input type=\"hidden\" id=\"\" name=\"scId\" value=\"dbc40591-0945-45ee-8fb4-e8018aef23d3\">
        <ul>
            <li>
                <h3 class=\"account-details__option-title\">Personal Information</h3>
            </li>
            <li class=\"js-text-info account-details__icon-container\">
                <p class=\"account-details__info-text js-name-selector\">JOHN DOE</p>
            </li>
            <li class=\"coned-form__field-container hidden js-input-info\">
                <div class=\"date-box date-box--no-padding\">
                    <h4 class=\"date-box__title\">Customer Name</h4>
                    <p class=\"date-box__date js-account-info-container\">JOHN DOE</p>
                </div>
            </li>
            <li class=\"js-text-info account-details__icon-container\">
                <p class=\"account-details__info-text js-email-selector\">email@email.com</p>
            </li>
            <li class=\"coned-form__field-container hidden js-input-info \">

<div class=\"coned-field\">
    <input id=\"mailing-address-name\" 
           name=\"mailingAddressName\" 
           class=\"coned-input js-coned-input js-input-name coned-input--filled js-address-input \" 
           type=\"text\"
           
           minlength=\"\" maxlength=\"100\" 
           required=\"\" data-msg-required=\"Please enter a name.\" 
           data-msg=\"Please enter a valid name.\" 
           data-rule-addressInput='true'  
           data-saved-value='JOHN DOE' data-service-value='JOHN DOE'
           
    />
    <label class=\"coned-label js-coned-label\" for=\"mailing-address-name\">Name</label>

    <span class=\"border-bar js-border-bar-selector\"></span>
</div>
            </li>
            <li class=\"js-text-info account-details__icon-container\">
                <p class=\"account-details__info-text js-service-address\">1234 STREET STREET</p>
            </li>
            <li class=\"coned-form__field-container hidden js-input-info \">
)";

std::string httpResponse(websitedata);

int coned_self_test() {
  ConEdScraper cescraper;
  std::string name = cescraper.parse_name_response(websitedata);
  std::string address = cescraper.parse_address_response(websitedata);
  std::string input1("ISTHIRTYONECHARACTERSLONGSTRING");
  commit(input1.c_str(), input1.c_str());
  // ConEdScraper cescraper;
  // int res;
  // res = cescraper.perform_query("placeholder"); // sub password here
  // std::string res2;
  return 0;
}


