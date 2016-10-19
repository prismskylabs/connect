# Prism Connect SDK

The Prism Connect SDK is a first class client library for the Prism Connect API. In addition to support for making all of the necessary network calls to the API, the SDK provides a standard suite of abstracted computer vision algorithms for interacting most effectively with the API. Finally, the SDK comes with a curated set of small example executables that can be used to build more complex applications, and they double as utilities for interacting with the API from the command line.

## Design

Simplicity, modularity, and flatness inform the design of the SDK. Here's how you query for all of the Prism accounts that can be accessed through a particular API token:

```c++
#include <iostream>

#include <api/client.h>
#include <api/environment.h>

int main() {
    prism::connect::api::Client client{prism::connect::api::environment::ApiRoot,
                                       prism::connect::api::environment::ApiToken};

    auto accounts = client.QueryAccounts();

    for (const auto& account : accounts) {
        std::cout << "Account[" << account.id_ << "]:" << std::endl;
        std::cout << "Name: " << account.name_ << std::endl;
        std::cout << "Url: " << account.url_ << std::endl;
        std::cout << "Instruments Url: " << account.instruments_url_ << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
```

Similarly, performing background subtraction on a sequence of input images should be straightforward and fool-proof:

```c++
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>

#include <processors/background.h>

int main() {
    prism::connect::processors::Background background;

    std::vector<cv::Mat> input_images;

    for (const auto& input_image : input_images) {
        background.AddImage(input_image);
    }

    cv::Mat background_image = background.GetBackgroundImage();
    cv::Mat foreground_mask = background.GetForegroundMask();

    return 0;
}
```

The goal of the SDK is to reduce the number of steps from the surface (aka `int main()`) to the core functionality of processing visual data and POSTing it to the Prism Connect API. This enables and rewards producing small, highly focused and functional executables that perform a single job correctly and efficiently, a critical property of successful embedded development workflows.

## Modules

The SDK is broken up into modules that are each responsible for a specific set of behaviors.

### `api`

To make network calls to the Prism Connect API, use the `api` module, which encapsulates authentication and network services in a single class, the `Client`. Every public method of an instance of `Client` is a network call, with no exceptions. This makes it easy to identify in source code what method calls must be treated as potentially blocking or erroneous.

After successfully constructing a `Client`, one has access to these methods:

```c++
// Get the list of Accounts
std::vector<Account> accounts = client.QueryAccounts();

// Get a specific Account by account id
Account account = client.QueryAccount(id);

// Get the list of Instruments belonging to an Account
std::vector<Instrument> client.QueryInstruments(account);

// Get a specific Instrument belonging to an Account by instrument id
Instrument client.QueryInstrument(account, id);

// Register an unregistered Instrument to an Account
client.RegisterInstrument(account, instrument);

// POST image, video, or time series data to a registered Instrument
client.PostImage(instrument, "ImageKey", std::chrono::system_clock::now(),
                 std::chrono::system_clock::now(), "image.jpg", image_data);
client.PostVideo(instrument, "VideoKey", std::chrono::system_clock::now(),
                 std::chrono::system_clock::now(), "video.mp4", video_data);
client.PostTimeSeries(instrument, "TimeSeriesKey", std::chrono::system_clock::now(),
                      std::chrono::system_clock::now(), json_data);
// In the above examples, image_data and video_data are std::vector<char>, aka, binary data, and
// json_data is type nlohmann::json, which is defined in an included dependency
```

The classes `Account` and `Instrument` are light classes with essentially all POD (plain old data) type members, and do not interact with the network on their own. Only when used in conjunction with a `Client` will a network call be made on behalf of them.
