#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "log.hpp"

#pragma once

template <class BufferType, class MessageType>
void ConvertBufferToMessage(BufferType buffer, MessageType &messageResult)
{
    {
        std::stringstream stream(buffer);
        boost::archive::text_iarchive ia(stream);

        ia >> messageResult;
    }
}

template <class MessageType>
std::string ConvertMessageToString(MessageType &message)
{
    std::stringstream stream("");
    {
        boost::archive::text_oarchive oa(stream);

        oa << message;
    }
    return stream.str();
}