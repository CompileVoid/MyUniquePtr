# MyUniquePtr

A lightweight, header-only C++17 implementation of an exclusive-ownership smart pointer (`std::unique_ptr` alternative) built from scratch with RAII compliance, move semantics, SFINAE-constrained polymorphic conversions, and unit testing.

## Features

* **Header-Only Library:** Easy to integrate into any C++17+ project via CMake `INTERFACE` target.
* **Strict Exclusive Ownership:** Copy constructor and copy assignment are explicitly deleted; ownership can only be transferred via move operations.
* **Polymorphic Conversions:** SFINAE-constrained (`std::enable_if_t<std::is_convertible_v<U*, T*>>`) converting move constructor and move assignment supporting `Derived` to `Base` pointer conversions.
* **Full Modifier Suite:** Implements `get()`, `release()`, `reset()`, `swap()`, and `explicit operator bool()`.
* **Factory & Helper Functions:** Includes variadic `makeUnique<T>(Args&&...)`, non-member `swap()`, and equality comparison operators (`==`, `!=`).
* **Modern Build & Testing Setup:** Configured with `CMakePresets.json`, `vcpkg` manifest mode, and comprehensive unit tests using **GoogleTest**.

## Project Structure

```text
MyUniquePtr/
├── include/
│   └── MyUniquePtr.h          # Smart pointer and helper functions implementation
├── tests/
│   └── test_my_unique_ptr.cpp # GoogleTest unit test suite
├── .gitignore                 # Git ignore rules for CMake, CLion, and vcpkg
├── CMakeLists.txt             # CMake build and test configuration
├── CMakePresets.json          # Ninja + clang-cl / vcpkg build presets
└── vcpkg.json                 # Dependency manifest (gtest)
```

## Building and Running Tests

1. **Configure the project using CMake Presets:**
   ```bash
   cmake --preset dev
   ```

2. **Build the test suite:**
   ```bash
   cmake --build --preset dev
   ```

3. **Run unit tests via CTest:**
   ```bash
   ctest --preset dev
   ```

## Usage Example

```cpp
#include <iostream>
#include <string>
#include "MyUniquePtr.h"

struct Base {
    virtual ~Base() = default;
    virtual void greet() const { std::cout << "Hello from Base\n"; }
};

struct Derived : Base {
    void greet() const override { std::cout << "Hello from Derived\n"; }
};

int main() {
    auto ptr = makeUnique<Derived>();
    ptr->greet();

    MyUniquePtr<Base> basePtr = std::move(ptr);
    if (!ptr && basePtr) {
        basePtr->greet();
    }

    return 0;
}
```
