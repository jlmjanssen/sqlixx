// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
// SPDX-License-Identifier: BSL-1.0

export module sqlixx:handles;

import std;

namespace sqlixx {

template <typename Handle, typename... Mixins>
    requires std::is_trivially_default_constructible_v<Handle> &&
             (std::is_trivially_default_constructible_v<Mixins> && ...)
class shallow_handle : public Mixins... {
public:
    constexpr shallow_handle() noexcept = default;
    explicit constexpr shallow_handle(Handle handle) noexcept : handle_(handle) {}

    [[nodiscard]] constexpr explicit operator bool() const noexcept { return handle_ != Handle{}; }
    [[nodiscard]] constexpr auto get() const noexcept -> Handle { return handle_; }

private:
    Handle handle_{};
};

template <typename Handle, auto Deleter, typename... Mixins>
    requires std::is_trivially_default_constructible_v<Handle> &&
             (noexcept(std::invoke(Deleter, std::declval<Handle>()))) &&
             (std::is_trivially_default_constructible_v<Mixins> && ...)
class owning_handle : public Mixins... {
public:
    using shallow_handle_type = shallow_handle<Handle, Mixins...>;

    constexpr owning_handle() noexcept = default;
    explicit constexpr owning_handle(Handle handle) noexcept : handle_(handle) {}

    owning_handle(const owning_handle&) = delete;
    auto operator=(const owning_handle&) -> owning_handle& = delete;

    constexpr owning_handle(owning_handle&& other) noexcept : handle_(other.release()) {}
    constexpr auto operator=(owning_handle&& other) noexcept -> owning_handle& {
        if (this != &other) [[likely]] {
            owning_handle deallocator{release()};
            handle_ = other.release();
        }
        return *this;
    }

    constexpr ~owning_handle() noexcept {
        if (Handle handle = release(); handle != Handle{}) {
            std::invoke(Deleter, handle);
        }
    }

    constexpr auto release() noexcept -> Handle { return std::exchange(handle_, Handle{}); }

    [[nodiscard]] constexpr explicit operator bool() const noexcept { return handle_ != Handle{}; }
    [[nodiscard]] constexpr auto get() const noexcept -> Handle { return handle_; }
    [[nodiscard]] constexpr operator shallow_handle_type() const noexcept { return shallow_handle_type{handle_}; }

private:
    Handle handle_{};
};

} // namespace sqlixx
