#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#  include <unistd.h>
#endif
#include <curl/curl.h>
#include "./messageManagerClass/messageManagerClass.h"

MessageHandler queueManager("localhost" , 5672 ,"guest" ,"guest" , "test" , "downloaderA_result");

#define MAX_PARALLEL 5 /* number of simultaneous transfers */
//#define NUM_URLS  1 //sizeof(urls)/sizeof(char *)
#define PENDING_LINKS 10
bool j;
const char *urls[PENDING_LINKS];
std::string result;
void fill_the_Url_list (int index){
    //std::cout<<index<<std::endl;
    std::string receivedUrl = queueManager.receiveMessage();
    std::cout<<receivedUrl<<std::endl;
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
    //std::cout<<urls[index]<<std::endl;
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
    //std::cout<<"URL which will go to dow :: " <<urls[i]<<std::endl;
    std::string URL = urls[i];
    std::cout << "URL before setting: " << URL << std::endl;
    curl_easy_setopt(eh, CURLOPT_URL,  urls[i]);
    curl_easy_setopt(eh, CURLOPT_PRIVATE,  urls[i]);
    curl_multi_add_handle(cm, eh);
    (*left)++;
    //std::cout<<"go to update list "<<std::endl;

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
    //std::cout<<"done with create trasfer"<<std::endl;
//    for(int i = 0 ; i < NUM_URLS ; i ++ ){
//        std::cout<<urls[i]<<std::endl;
//    }

    do {
        int still_alive = 1;
        curl_multi_perform(cm, &still_alive);

        while((msg = curl_multi_info_read(cm, &msgs_left))) {
            if(msg->msg == CURLMSG_DONE) {
                char *url;
                char *url2;
                CURL *e = msg->easy_handle;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
                curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url2);
                std::cout<<"this is downloaded url : " << url2   << std::endl ;
                //printf("URL: %s\n", url);
                std::cout << "R: " << msg->data.result << " - " << curl_easy_strerror(msg->data.result) << " <" << url << ">" << std::endl;
                result = std::to_string(msg->data.result) + "~" + url;
                std::cout<<"----------------"<<result << std::endl;
                queueManager.sendMessage(result);
                curl_multi_remove_handle(cm, e);
                curl_easy_cleanup(e);
                left--;
                fill_the_Url_list(transfers);
            }
            else {
                fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
            }
            if(j) {
                add_transfer(cm, (transfers++) % PENDING_LINKS , &left);
                transfers %= PENDING_LINKS; // Ensure transfers wraps around
            }
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
