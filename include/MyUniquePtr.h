#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T>
class MyUniquePtr
{
public:
    using pointer = T*;
    using element_type = T;

    constexpr MyUniquePtr() noexcept = default;
    constexpr MyUniquePtr(std::nullptr_t) noexcept : ptr_(nullptr) {}
    explicit MyUniquePtr(pointer ptr) noexcept : ptr_(ptr) {}

    MyUniquePtr(const MyUniquePtr&) = delete;
    MyUniquePtr& operator=(const MyUniquePtr&) = delete;

    MyUniquePtr(MyUniquePtr&& other) noexcept : ptr_(std::move(other.ptr_)) { other.ptr_ = nullptr; }
    MyUniquePtr& operator=(MyUniquePtr&& other) noexcept
    {
        if (this != &other)
        {
            reset(other.release());
        }
        return *this;
    }
    MyUniquePtr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    MyUniquePtr(MyUniquePtr<U>&& other) noexcept : ptr_(other.release())
    {
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    MyUniquePtr& operator=(MyUniquePtr<U>&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    ~MyUniquePtr()
    {
        if (ptr_)
        {
            delete ptr_;
        }
    }

    [[nodiscard]] pointer get() const noexcept { return ptr_; }
    pointer release() noexcept
    {
        pointer tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }
    void reset(pointer ptr = nullptr) noexcept
    {
        pointer old = ptr_;
        ptr_ = ptr;
        delete old;
    }
    void swap(MyUniquePtr& other) noexcept { std::swap(ptr_, other.ptr_); }

    [[nodiscard]] explicit operator bool() const noexcept { return ptr_ != nullptr; }
    T& operator*() const noexcept { return *ptr_; }
    pointer operator->() const noexcept { return ptr_; }

private:
    pointer ptr_{nullptr};
};

template <typename T, typename... Args>
[[nodiscard]] MyUniquePtr<T> makeUnique(Args&&... args)
{
    return MyUniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
void swap(MyUniquePtr<T>& lhs, MyUniquePtr<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <typename T, typename U>
[[nodiscard]] bool operator==(const MyUniquePtr<T>& lhs, const MyUniquePtr<U>& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

template <typename T, typename U>
[[nodiscard]] bool operator!=(const MyUniquePtr<T>& lhs, const MyUniquePtr<U>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T>
[[nodiscard]] bool operator==(const MyUniquePtr<T>& lhs, std::nullptr_t) noexcept
{
    return !lhs;
}

template <typename T>
[[nodiscard]] bool operator!=(const MyUniquePtr<T>& lhs, std::nullptr_t) noexcept
{
    return static_cast<bool>(lhs);
}
