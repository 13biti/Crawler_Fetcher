#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <iostream>

using namespace std;

constexpr auto queueName = "PublisherQueue";

int main()
{
    try
    {
        auto channel = AmqpClient::Channel::Create();
        std::pair<std::string, int> ttl_message("x-message-ttl", 60000); // 1 minute
        std::pair<std::string, int> max_message("x-max-length", 100); // only 100 message at time in queue
        std::pair<std::string, std::string> x_overflow("x-overflow", "drop-head");
        channel->DeclareQueue(queueName, false, true, true, false, {ttl_message, max_message, x_overflow});
        auto consumerTag = channel->BasicConsume(queueName);
        clog << "Consumer tag: " << consumerTag << endl;
        
        while (true)
        {
            auto envelope = channel->BasicConsumeMessage(consumerTag);
            clog << " [x] Received " << envelope->Message()->Body() << endl;
            channel->BasicAck(envelope);
        }
    }
    catch (const exception& error)
    {
        cerr << error.what() << endl;
    }

    return 0;
}
/*
#pragma warning(push, 0)
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#pragma warning(pop)

#include <iostream>

using namespace std;

constexpr auto QUEUE_NAME = "PublisherQueue";
int main()
{
  try
  {
    auto channel = AmqpClient::Channel::Create();
    channel->DeclareQueue(QUEUE_NAME, false, true, false, true);
    auto consumerTag = channel->BasicConsume(QUEUE_NAME);
    clog << "Consumer tag: " << consumerTag << endl;
    auto envelop = channel->BasicConsumeMessage(consumerTag);
    clog << " [x] Received " << envelop->Message()->Body() << endl;
  }
  catch (const exception& error)
  {
    cerr << error.what() << endl;
  }

  return 0; 
}
*/
