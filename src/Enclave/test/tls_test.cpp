#include <Debug.h>
#include "tls_client.h"
#include "Log.h"
#include <stdexcept>
#include <trusted_ca_certs.h>

const char* ref_page = \
    "<!DOCTYPE html>\n"
    "<html><head>\n"
    "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
    "  </head>\n"
    "  <body>\n"
    "      <h1>Herman Melville - Moby-Dick</h1>\n"
    "\n"
    "      <div>\n"
    "        <p>\n"
    "          Availing himself of the mild, summer-cool weather that now \n"
    "reigned in these latitudes, and in preparation for the peculiarly active\n"
    " pursuits shortly to be anticipated, Perth, the begrimed, blistered old \n"
    "blacksmith, had not removed his portable forge to the hold again, after \n"
    "concluding his contributory work for Ahab's leg, but still retained it \n"
    "on deck, fast lashed to ringbolts by the foremast; being now almost \n"
    "incessantly invoked by the headsmen, and harpooneers, and bowsmen to do \n"
    "some little job for them; altering, or repairing, or new shaping their \n"
    "various weapons and boat furniture. Often he would be surrounded by an \n"
    "eager circle, all waiting to be served; holding boat-spades, pike-heads,\n"
    " harpoons, and lances, and jealously watching his every sooty movement, \n"
    "as he toiled. Nevertheless, this old man's was a patient hammer wielded \n"
    "by a patient arm. No murmur, no impatience, no petulance did come from \n"
    "him. Silent, slow, and solemn; bowing over still further his chronically\n"
    " broken back, he toiled away, as if toil were life itself, and the heavy\n"
    " beating of his hammer the heavy beating of his heart. And so it \n"
    "was.—Most miserable! A peculiar walk in this old man, a certain slight \n"
    "but painful appearing yawing in his gait, had at an early period of the \n"
    "voyage excited the curiosity of the mariners. And to the importunity of \n"
    "their persisted questionings he had finally given in; and so it came to \n"
    "pass that every one now knew the shameful story of his wretched fate. \n"
    "Belated, and not innocently, one bitter winter's midnight, on the road \n"
    "running between two country towns, the blacksmith half-stupidly felt the\n"
    " deadly numbness stealing over him, and sought refuge in a leaning, \n"
    "dilapidated barn. The issue was, the loss of the extremities of both \n"
    "feet. Out of this revelation, part by part, at last came out the four \n"
    "acts of the gladness, and the one long, and as yet uncatastrophied fifth\n"
    " act of the grief of his life's drama. He was an old man, who, at the \n"
    "age of nearly sixty, had postponedly encountered that thing in sorrow's \n"
    "technicals called ruin. He had been an artisan of famed excellence, and \n"
    "with plenty to do; owned a house and garden; embraced a youthful, \n"
    "daughter-like, loving wife, and three blithe, ruddy children; every \n"
    "Sunday went to a cheerful-looking church, planted in a grove. But one \n"
    "night, under cover of darkness, and further concealed in a most cunning \n"
    "disguisement, a desperate burglar slid into his happy home, and robbed \n"
    "them all of everything. And darker yet to tell, the blacksmith himself \n"
    "did ignorantly conduct this burglar into his family's heart. It was the \n"
    "Bottle Conjuror! Upon the opening of that fatal cork, forth flew the \n"
    "fiend, and shrivelled up his home. Now, for prudent, most wise, and \n"
    "economic reasons, the blacksmith's shop was in the basement of his \n"
    "dwelling, but with a separate entrance to it; so that always had the \n"
    "young and loving healthy wife listened with no unhappy nervousness, but \n"
    "with vigorous pleasure, to the stout ringing of her young-armed old \n"
    "husband's hammer; whose reverberations, muffled by passing through the \n"
    "floors and walls, came up to her, not unsweetly, in her nursery; and so,\n"
    " to stout Labor's iron lullaby, the blacksmith's infants were rocked to \n"
    "slumber. Oh, woe on woe! Oh, Death, why canst thou not sometimes be \n"
    "timely? Hadst thou taken this old blacksmith to thyself ere his full \n"
    "ruin came upon him, then had the young widow had a delicious grief, and \n"
    "her orphans a truly venerable, legendary sire to dream of in their after\n"
    " years; and all of them a care-killing competency.\n"
    "        </p>\n"
    "      </div>\n"
    "  \n"
    "</body></html>";

int ssl_self_test(){
    HttpRequest request("s3.amazonaws.com", "/tc-travis/html.htm", true);
    HttpsClient client(request);

  LL_INFO("%d CA loaded", root_cas_pem_len);

    try {
        HttpResponse response = client.getResponse();
        if (0 != string(ref_page).compare(response.getContent())) {
            return -1;
        }
    }

    catch (std::runtime_error& e)
    {
        LL_CRITICAL("%s", e.what());
        LL_CRITICAL("%s", client.getError().c_str());
        return -1;
    }

    return 0;
}
