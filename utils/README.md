# ovis::utils
This module contains general C++ utilities.
The following sections contain the most important aspects of the module.

[Error Handling](#error-handling)

## Error Handling
The header [result.hpp](utils/include/ovis/utils/result.hpp) contains a the `Result<T, E>` class which is used within the engine for error handling instead of error codes or exceptions.
`Result<T, E>` works similar to the proposed [`std::expected<T, E>`](https://en.cppreference.com/w/cpp/utility/expected) or [`Result<T, E>`](https://doc.rust-lang.org/std/result/) in Rust.
In addition, the header defines a simple `Error` which simply stores an error message.
This error type is sufficient for most use cases where you do not want to differentiate between different errors that can occur during a function call.
`Error` has a constructor that takes a format string and its corresponding parameters to produce the final message using [{fmt}](https://github.com/fmtlib/fmt) (look below for an example).

### Usage
`Result<T, E>` is intended to be used as a return type for functions.
`T` should be the type the function should return on success (using `void` for functions that do not produce a value is completely fine).
`E` should be the error type, which defaults to the simple `Error` struct described above.

E.g., a function that reads the whole content of a text file could look like this:
```C++
Result<std::string> ReadFile(const std::filesystem::path& path) {
  std::ifstream file(path);

  if (!file) {
    return Error("Cannot open file: {}", path);
  }

  file.seekg(0, std::ios::end);
  std::string content(file.tellg(), '\0');
  file.seekg(0, std::ios::beg);
  if (!file.read(content.data(), content.size())) {
    return Error("Could not read file: {}", path);
  }

  return std::move(content);
}
```
Then you would could use the function like this:
```C++
if (auto content = ReadFile(some_path); content.has_value()) {
  // Instead of content.has_value(), you could simply write content, as the bool operator is overloaded
  
  // Result<T, E> behaves a little bit like std::optional or a smart pointer in the sense that the arrow
  // operator (content->) as well as dereferencing the result will give access to the underlying T.
  print("The content of the file is: {}", *content);
} else {
  print("Oh no, an error occured: {}", content.error().message);
}
```

### Usage of `void` Results
The Result class is specialized for `Result<void, E>`, which basically results in and `std::optional<E>`.
`Result<void, E>` does not have a has_value() method, but the `bool` operator is still overloaded to return true if the function succeeded and false if an error occured.
Similarly, it does not have the arrow and dereference operator overloaded, as this would not make sense in this context.
Constructing a successful `Result<void, E>` is also strange, as there is no `T` we can construct it with.
First, a default constructed `Result<void, E>` would indicate success, however, this was confusing so the `SuccessType` struct was introduced.
Similarly, to `std::nullopt_t` its only use is to construct a `Result<void, E>` that indicates success.
For convenience, the global declaration `constexpr SuccessType Success;` exists to allow simple construction of such types.
E.g.,:
```C++
Result<void> Sleep(int time) {
  if (time < 0) {
    return Error("Cannot travel in time");
  }
  SleepInternal(time);
  return Success;
}
```
### Convenience

### Caveats
