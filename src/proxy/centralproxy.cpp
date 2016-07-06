/**
 *   (C) Copyright 2016 Mirantis, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */


#include "centralproxy.h"

#include <easylogging++.h>
#include <zmq_addon.hpp>

#include "common/configuration.h"
#include "common/matchmaker.h"
#include "common/utils.h"


using namespace zmqproxy;


CentralProxy::CentralProxy(const Configuration& conf, Matchmaker& matchmaker)
    : m_conf(conf),
      m_matchmaker(matchmaker),
      m_context(zmq::context_t(3)),
      m_feRouter(zmq::socket_t(m_context, zmq::socket_type::router)),
      m_beRouter(zmq::socket_t(m_context, zmq::socket_type::router)),
      m_publisher(zmq::socket_t(m_context, zmq::socket_type::pub)),
      m_fePort(35001),
      m_bePort(35002),
      m_publisherPort(35003),
      m_feRouterAddress(conf.host() + ":" + std::to_string(m_fePort)),
      m_beRouterAddress(conf.host() + ":" + std::to_string(m_bePort)),
      m_publisherAddress(conf.host() + ":" + std::to_string(m_publisherPort))
{
    el::Loggers::getLogger("CentralProxy");
    CLOG(INFO, "CentralProxy") << "Starting central router ";

    CLOG(INFO, "CentralProxy") << "Bind to address: " << "tcp://*:" + std::to_string(m_fePort);
    m_feRouter.bind("tcp://*:" + std::to_string(m_fePort));
    CLOG(INFO, "CentralProxy") << "Bind to address: " << "tcp://*:" + std::to_string(m_bePort);
    m_beRouter.bind("tcp://*:" + std::to_string(m_bePort));
    CLOG(INFO, "CentralProxy") << "Bind to address: " << "tcp://*:" + std::to_string(m_publisherPort);
    m_publisher.bind("tcp://*:" + std::to_string(m_publisherPort));

    m_matchmaker.registerPublisher(std::make_pair(m_publisherAddress, m_feRouterAddress));
    m_matchmaker.registerRouter(m_beRouterAddress);
}


CentralProxy::~CentralProxy()
{
    CLOG(INFO, "CentralProxy") << "Destroy central router ";
    m_matchmaker.unregisterPublisher(std::make_pair(m_publisherAddress, m_feRouterAddress));
    m_matchmaker.unregisterRouter(m_beRouterAddress);
}


void CentralProxy::pollForMessages()
{
    //  Initialize poll set
    zmq::pollitem_t items [] = {
        { static_cast<void *>(m_feRouter), 0, ZMQ_POLLIN, 0 },
        { static_cast<void *>(m_feRouter), 0, ZMQ_POLLIN, 0 }
    };
    //  Process messages from both sockets
    while (1) {
        zmq::poll (&items [0], 2, -1);

        if (items [0].revents & ZMQ_POLLIN) {
            redirectInRequest(m_feRouter, m_beRouter);
        }
        if (items [1].revents & ZMQ_POLLIN) {
            redirectInRequest(m_beRouter, m_feRouter);
        }
    }
}


void CentralProxy::redirectInRequest(zmq::socket_t& socketFe, zmq::socket_t& socketBe)
{
//    CLOG(DEBUG, "CentralProxy") << "Message pending ";

    zmq::message_t replyId;
    socketFe.recv(&replyId);

    zmq::message_t empty;
    socketFe.recv(&empty);

    zmq::message_t msgType;
    socketFe.recv(&msgType);
    auto messageType = getMessageType(msgType);
//    CLOG(DEBUG, "CentralProxy") << "Message type: " << toString(messageType);

    if (isDirect(messageType))
    {
//        CLOG(DEBUG, "CentralProxy") << "Direct type handling";

        zmq::message_t routingKey;
        socketFe.recv(&routingKey);
        socketBe.send(routingKey, ZMQ_SNDMORE);

        zmq::message_t routingKey;
        sendString(socketBe, "", ZMQ_SNDMORE);
        socketBe.send(replyId, ZMQ_SNDMORE);
        socketBe.send(msgType, ZMQ_SNDMORE);

//        auto routingKey = getString(message.pop());
//        CLOG(DEBUG, "CentralProxy") << "Routing key: " << routingKey;
//        auto messageId = getString(message.pop());
//        CLOG(DEBUG, "CentralProxy") << "Dispatching message: " << messageId;

        sendString(socketBe, routingKey, ZMQ_SNDMORE);
        sendString(socketBe, "", ZMQ_SNDMORE);
        socketBe.send(replyId, ZMQ_SNDMORE);
        socketBe.send(msgType, ZMQ_SNDMORE);

        zmq::message_t messageId;
        socketFe.recv()

        sendString(socketBe, messageId, ZMQ_SNDMORE);
        message.send(socketBe);
    }
    else if (isMultisend(messageType))
    {
        CLOG(DEBUG, "CentralProxy") << "Multisend type handling";
    }
}
