// https://www.oschina.net/translate/go-defer-in-cpp
// util/scopeguard.hpp
#pragma once

#include <utility>
#include <functional>

namespace sss {

class scopeguard {
public:
    template<class Callable>
    scopeguard(Callable &&fn) : fn_(std::forward<Callable>(fn)) {}
    scopeguard(scopeguard &&other) : fn_(std::move(other.fn_)) {
        other.fn_ = nullptr;
    }
    ~scopeguard() {
        // NOTE must not throw
        if (fn_) fn_();
    }
    scopeguard(const scopeguard &) = delete;
    scopeguard& operator=(const scopeguard&) = delete;

private:
    std::function<void()> fn_;
};

} // namespace sss
