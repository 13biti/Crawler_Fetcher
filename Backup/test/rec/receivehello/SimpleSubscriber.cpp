#include "SimpleSubscriber.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace AmqpClient
{

SimpleSubscriber::SimpleSubscriber(Channel::ptr_t channel, const std::string &publisher_name) :
    m_channel(channel)
{
    m_consumerQueue = "SimpleSubscriber_";
   // boost::uuids::random_generator uuid_gen;
   // boost::uuids::uuid guid(uuid_gen());
    //m_consumerQueue += boost::lexical_cast<std::string>(guid);

    m_channel->DeclareQueue(m_consumerQueue);
    m_channel->BindQueue(m_consumerQueue, publisher_name,"");

    m_channel->BasicConsume(m_consumerQueue, m_consumerQueue);
}

SimpleSubscriber::~SimpleSubscriber()
{
}

std::string SimpleSubscriber::WaitForMessageString(int timeout)
{
    BasicMessage::ptr_t incoming = WaitForMessage(timeout);
    return incoming->Body();
}

BasicMessage::ptr_t SimpleSubscriber::WaitForMessage(int timeout)
{
    Envelope::ptr_t envelope;
    m_channel->BasicConsumeMessage(m_consumerQueue, envelope, timeout);
    return envelope->Message();
}

} //namespace AmqpClient
