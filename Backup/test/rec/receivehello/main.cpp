#include <stdlib.h>
#include <stdio.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <iostream>
#include "SimpleSubscriber.h"
#include <boost/make_shared.hpp>

using namespace AmqpClient;
using namespace std;
int main()
{
    const char *szBroker = (getenv("AMQP_BROKER") != NULL) ? getenv("AMQP_BROKER") :"PublisherQueue";
    //char *szBroker = (getenv("AMQP_BROKER") != NULL) ? getenv("AMQP_BROKER") : "PublisherQueue";
    Channel::ptr_t channel;
    //    SimpleSubscriber pub(channel,"wt");
    boost::shared_ptr<SimpleSubscriber> pub=SimpleSubscriber::Create(channel, "wt");
    
	while (1)
	{
		string a;
		//  a= pub.WaitForMessageString(5000);

		Envelope::ptr_t envelope;
		bool flag = channel->BasicConsumeMessage("SimpleSubscriber_", envelope, 7000);
		if (flag == false)
		{
			cout << "timeout\n" << endl;
			break;
		}

		a = envelope->Message()->Body();

		cout << a << endl;
		
	}

}

