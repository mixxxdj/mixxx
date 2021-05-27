/* Copyright 2016, Ableton AG, Berlin. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  If you would like to incorporate Link into a proprietary software application,
 *  please contact <link-devs@ableton.com>.
 */

#pragma once

#include <ableton/util/Log.hpp>

namespace ableton
{
namespace discovery
{
namespace test
{

class Interface
{
public:
  void send(const uint8_t* const bytes,
    const size_t numBytes,
    const asio::ip::udp::endpoint& endpoint)
  {
    sentMessages.push_back(
      std::make_pair(std::vector<uint8_t>{bytes, bytes + numBytes}, endpoint));
  }

  template <typename Callback, typename Tag>
  void receive(Callback callback, Tag tag)
  {
    mCallback = [callback, tag](const asio::ip::udp::endpoint& from,
                  const std::vector<uint8_t>& buffer) {
      callback(tag, from, begin(buffer), end(buffer));
    };
  }

  template <typename It>
  void incomingMessage(
    const asio::ip::udp::endpoint& from, It messageBegin, It messageEnd)
  {
    std::vector<uint8_t> buffer{messageBegin, messageEnd};
    mCallback(from, buffer);
  }

  asio::ip::udp::endpoint endpoint() const
  {
    return asio::ip::udp::endpoint({}, 0);
  }

  using SentMessage = std::pair<std::vector<uint8_t>, asio::ip::udp::endpoint>;
  std::vector<SentMessage> sentMessages;

private:
  using ReceiveCallback =
    std::function<void(const asio::ip::udp::endpoint&, const std::vector<uint8_t>&)>;
  ReceiveCallback mCallback;
};

} // namespace test
} // namespace discovery
} // namespace ableton
