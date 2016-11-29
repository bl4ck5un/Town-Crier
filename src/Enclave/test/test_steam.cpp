#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "SSLClient.h"
#include "Scraper_lib.h"
#include "dispatcher.h"
#include "Log.h"

int steam_self_test() {
    int rc, ret;
    const char * listB[1] = {"Portal"};
    char * test2[2] = {"Dark Ranger's Headdress", "Death Shadow Bow"};
    LL_NOTICE("Starting...\n");
    // needs as input time of request, T_B time for response, key, account # ID_B, list of items L_B, account # ID_S 
    // I'm not sure how we get time... but so long as we get it somehow...
    rc = get_steam_transaction(listB, 1, "32884794", 1456380265, "7978F8EDEF9695B57E72EC468E5781AD", &ret);
    LL_NOTICE("%d, %d\n", rc, ret);
    //rc should be 0, ret should be 1

    return 0;
}

