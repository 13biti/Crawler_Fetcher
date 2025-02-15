#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#  include <unistd.h>
#endif
#include <curl/curl.h>
#include "QueueManager.h"

QueueManager queueManager("localhost" , 5672);
#define MAX_PARALLEL 5 /* number of simultaneous transfers */
#define NUM_URLS  10 //sizeof(urls)/sizeof(char *)
#define PENDING_LINKS 10
bool j;
const char *urls[PENDING_LINKS]; /*= {
  "https://www.microsoft.com",
  "https://opensource.org",
  "https://www.google.com",
  "https://www.yahoo.com",
  "https://www.ibm.com",
  "https://www.mysql.com",
  "https://www.oracle.com",
  "https://www.ripe.net",
  "https://www.iana.org",
  "https://www.amazon.com",
  "https://www.netcraft.com",
  "https://www.heise.de",
  "https://www.chip.de",
  "https://www.ca.com",
  "https://www.cnet.com",
  "https://www.mozilla.org",
  "https://www.cnn.com",
  "https://www.wikipedia.org",
  "https://www.dell.com",
  "https://www.hp.com",
  "https://www.cert.org",
  "https://www.mit.edu",
  "https://www.nist.gov",
  "https://www.ebay.com",
  "https://www.playstation.com",
  "https://www.uefa.com",
  "https://www.ieee.org",
  "https://www.apple.com",
  "https://www.symantec.com",
  "https://www.zdnet.com",
  "https://www.fujitsu.com/global/",
  "https://www.supermicro.com",
  "https://www.hotmail.com",
  "https://www.ietf.org",
  "https://www.bbc.co.uk",
  "https://news.google.com",
  "https://www.foxnews.com",
  "https://www.msn.com",
  "https://www.wired.com",
  "https://www.sky.com",
  "https://www.usatoday.com",
  "https://www.cbs.com",
  "https://www.nbc.com/",
  "https://slashdot.org",
  "https://www.informationweek.com",
  "https://apache.org",
  "https://www.un.org",
};
*/

void fill_the_Url_list (int index){
    std::cout<<index<<std::endl;
    std::string receivedUrl = queueManager.receiveMessage("test");
    if (receivedUrl.empty()) {
        std::cerr << "Received empty URL." << std::endl;
        j = false;
        return;
    }

    size_t firstNonSpace = receivedUrl.find_first_not_of(" \t");
    receivedUrl = receivedUrl.substr(firstNonSpace);

    char* urlCopy = new char[receivedUrl.length() + 1];
    strcpy(urlCopy, receivedUrl.c_str());
    delete[] urls[index];
    urls[index] = urlCopy;
    j = true;
    std::cout<<urls[index]<<std::endl;
    //urls[index] = queueManager.receiveMessage("test").c_str();
}

static size_t write_cb(char *data, size_t n, size_t l, void *userp)
{
    /* take care of the data here, ignored in this example */
    (void)data;
    (void)userp;
    return n*l;
}

static void add_transfer(CURLM *cm, unsigned int i, int *left)
{
    CURL *eh = curl_easy_init();
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
    std::cout<<"URL which will go to dow :: " <<urls[i]<<std::endl;
    curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
    curl_multi_add_handle(cm, eh);
    (*left)++;
    std::cout<<"go to update list "<<std::endl;
    fill_the_Url_list(i);
}

int main(void)
{
    CURLM *cm;
    CURLMsg *msg;
    unsigned int transfers;
    int msgs_left = -1;
    int left = 0;
    short counter = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    cm = curl_multi_init();
    for(int i = 0 ; i < PENDING_LINKS ; i++ ){
        fill_the_Url_list(i);
    }
/*


    int x = 0;
    do{
        if (x == 5)
            x = 0;
        std::cout << urls[x]<<std::endl;
        x++;
    } while (j);

  /* Limit the amount of simultaneous connections curl should allow:
   */
    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL);

    for(transfers = 0; transfers < MAX_PARALLEL && transfers < PENDING_LINKS;
        transfers++) {
        add_transfer(cm, transfers, &left);
        counter++;
    }
    std::cout<<"done with create trasfer"<<std::endl;
//    for(int i = 0 ; i < NUM_URLS ; i ++ ){
//        std::cout<<urls[i]<<std::endl;
//    }

    do {
        int still_alive = 1;
        curl_multi_perform(cm, &still_alive);

        while((msg = curl_multi_info_read(cm, &msgs_left))) {
            if(msg->msg == CURLMSG_DONE) {
                char *url;
                CURL *e = msg->easy_handle;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
                fprintf(stderr, "R: %d - %s <%s>\n",
                        msg->data.result, curl_easy_strerror(msg->data.result), url);
                curl_multi_remove_handle(cm, e);
                curl_easy_cleanup(e);
                left--;
            }
            else {
                fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
            }
            if(j)
                add_transfer(cm, transfers++ % PENDING_LINKS , &left);
            /*
            if(transfers ){

                counter++;
                if (counter >= PENDING_LINKS )
                    counter = 0 ;
                std::cout<<counter << std::endl;
                add_transfer(cm,counter , &left);
               */
        }
        if(left)
            curl_multi_wait(cm, NULL, 0, 1000, NULL);

    } while(left);

    curl_multi_cleanup(cm);
    curl_global_cleanup();

    return EXIT_SUCCESS;
}
