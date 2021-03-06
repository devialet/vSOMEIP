// Copyright (C) 2014-2016 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/udp_ext.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include <vsomeip/constants.hpp>
#include <vsomeip/defines.hpp>

#include "../include/endpoint_host.hpp"
#include "../include/endpoint_impl.hpp"
#include "../../logging/include/logger.hpp"

namespace vsomeip {

template<typename Protocol>
endpoint_impl<Protocol>::endpoint_impl(
        std::shared_ptr<endpoint_host> _host,
        endpoint_type _local,
        boost::asio::io_service &_io,
        std::uint32_t _max_message_size)
    : service_(_io),
      host_(_host),
      is_supporting_magic_cookies_(false),
      has_enabled_magic_cookies_(false),
      max_message_size_(_max_message_size),
      use_count_(0),
      sending_blocked_(false),
      local_(_local) {
}

template<typename Protocol>
endpoint_impl<Protocol>::~endpoint_impl() {
}

template<typename Protocol>
void endpoint_impl<Protocol>::enable_magic_cookies() {
    has_enabled_magic_cookies_ = is_supporting_magic_cookies_;
}

template<typename Protocol>
uint32_t endpoint_impl<Protocol>::find_magic_cookie(
        byte_t *_buffer, size_t _size) {
    bool is_found(false);
    uint32_t its_offset = 0xFFFFFFFF;
    if (has_enabled_magic_cookies_) {
        uint8_t its_cookie_identifier, its_cookie_type;

        if (is_client()) {
            its_cookie_identifier =
                    static_cast<uint8_t>(MAGIC_COOKIE_SERVICE_MESSAGE);
            its_cookie_type =
                    static_cast<uint8_t>(MAGIC_COOKIE_SERVICE_MESSAGE_TYPE);
        } else {
            its_cookie_identifier =
                    static_cast<uint8_t>(MAGIC_COOKIE_CLIENT_MESSAGE);
            its_cookie_type =
                    static_cast<uint8_t>(MAGIC_COOKIE_CLIENT_MESSAGE_TYPE);
        }

        do {
            its_offset++; // --> first loop has "its_offset = 0"
            if (_size > its_offset + 16) {
                is_found = (_buffer[its_offset] == 0xFF
                         && _buffer[its_offset + 1] == 0xFF
                         && _buffer[its_offset + 2] == its_cookie_identifier
                         && _buffer[its_offset + 3] == 0x00
                         && _buffer[its_offset + 4] == 0x00
                         && _buffer[its_offset + 5] == 0x00
                         && _buffer[its_offset + 6] == 0x00
                         && _buffer[its_offset + 7] == 0x08
                         && _buffer[its_offset + 8] == 0xDE
                         && _buffer[its_offset + 9] == 0xAD
                         && _buffer[its_offset + 10] == 0xBE
                         && _buffer[its_offset + 11] == 0xEF
                         && _buffer[its_offset + 12] == 0x01
                         && _buffer[its_offset + 13] == 0x01
                         && _buffer[its_offset + 14] == its_cookie_type
                         && _buffer[its_offset + 15] == 0x00);
            } else {
                break;
            }

        } while (!is_found);
    }

    return (is_found ? its_offset : 0xFFFFFFFF);
}

template<typename Protocol>
void endpoint_impl<Protocol>::join(const std::string &) {
}

template<typename Protocol>
void endpoint_impl<Protocol>::leave(const std::string &) {
}

template<typename Protocol>
void endpoint_impl<Protocol>::add_default_target(
        service_t, const std::string &, uint16_t) {
}

template<typename Protocol>
void endpoint_impl<Protocol>::remove_default_target(service_t) {
}

template<typename Protocol>
unsigned short endpoint_impl<Protocol>::get_local_port() const {
    return 0;
}

template<typename Protocol>
bool endpoint_impl<Protocol>::is_reliable() const {
    return false;
}

template<typename Protocol>
void endpoint_impl<Protocol>::increment_use_count() {
    use_count_++;
}

template<typename Protocol>
void endpoint_impl<Protocol>::decrement_use_count() {
    if (use_count_ > 0)
        use_count_--;
}

template<typename Protocol>
uint32_t endpoint_impl<Protocol>::get_use_count() {
    return use_count_;
}

// Instantiate template
#ifndef WIN32
template class endpoint_impl<boost::asio::local::stream_protocol>;
#endif
template class endpoint_impl<boost::asio::ip::tcp>;
template class endpoint_impl<boost::asio::ip::udp>;
template class endpoint_impl<boost::asio::ip::udp_ext>;

} // namespace vsomeip
