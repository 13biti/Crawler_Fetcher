
#include "SimplePublisher.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <string>
#include <sstream>

namespace AmqpClient
{
SimplePublisher::SimplePublisher(Channel::ptr_t channel, const std::string &publisher_name) :
    m_channel(channel), m_publisherExchange(publisher_name)
{
    if (m_publisherExchange == "")
    {
        m_publisherExchange = "amq.direct";
        boost::uuids::random_generator uuid_gen;
        boost::uuids::uuid guid(uuid_gen());
        m_publisherExchange += boost::lexical_cast<std::string>(guid);
    }
    m_channel->DeclareExchange(m_publisherExchange);
}

SimplePublisher::~SimplePublisher()
{
    //m_channel->DeleteExchange(m_publisherExchange, false);
}

void SimplePublisher::Publish(const std::string &message)
{
    BasicMessage::ptr_t outgoing_message = BasicMessage::Create();
    outgoing_message->Body(message);

    Publish(outgoing_message);
}

void SimplePublisher::Publish(const BasicMessage::ptr_t message)
{
    m_channel->BasicPublish(m_publisherExchange, "", message);
}

} // namespace AmqpClient
