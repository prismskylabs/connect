#ifndef PRISM_CONNECT_API_Client_H_
#define PRISM_CONNECT_API_Client_H_

#include <memory>
#include <string>

namespace prism {
namespace connect {
namespace api {

class Client {
  public:
    Client(const std::string& api_root, const std::string& api_token);
    ~Client();

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Client_H_
