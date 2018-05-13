## Result
A library to aid error handling in situations where
`throw`-ing is not appropriate. A type of `Result<T, E>` 
holds either a `T` or a `E`, `T` being the successful
type and `E` being the error type.

*License: MIT (see `LICENSE.txt`)*

### Example

```c++
#include "result/result.hpp"
#include <system_error>

auto may_result_in_error() -> result::Result<size_t, std::error_code> {
   
    auto e = read();
    if (e) {
        return result::err(std::error_code{e, std::system_category()});
    }

    return result::ok(e);
}

auto main(int, char const**) -> int {

    auto r = may_result_in_error();
    if (r.is_ok()) {
        std::cout << "Read " << r.value() << " bytes\n";
    }
    else {
        std::cerr << "Error: " 
            << r.error().value()
            << "\n";
    }

    return 0;
}
```
