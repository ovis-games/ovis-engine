# ovis::utils
This module contains general C++ utilities.
The following sections contain the most important aspects of the module.

* [Ranges](#ranges)
* [Error Handling](#error-handling)

## Ranges
The header [range.hpp](utils/include/ovis/utils/result.hpp) contains a lot of helper functions regarding ranges.
Most of the concepts have made its way into the C++ 20 standard in the new `ranges` library and they should be used instead.

#### IntegralRange
Generate an integer range, similar to [`std::ranges::iota_view`](https://en.cppreference.com/w/cpp/ranges/iota_view).
Usage:
```C++
for (auto i : IntegralRange<int>{0, 3}) {
  // i will have the values 0, 1, 2
}
// or:
for (auto i : IRange(3)) {
  // in contrast to std::views::iota(), calling IRange with a single argument
  // it denotes the end of the range that starts at zero, so this also iterates
  // over the values 0, 1, 2
}
```

#### IndexedRange
Equivalent of [`std::views::enumerate`](https://en.cppreference.com/w/cpp/ranges/enumerate_view).
```C++
std::vector<std::string> strings = { "foo", "bar" };
for (auto string : IndexRange(strings)) {
  // string.value() or string-> accesses the underlying value
  // string.index() returns the zero-based index
}
```

#### RangeFilter
Equivalent of [`std::views::filter`](https://en.cppreference.com/w/cpp/ranges/filter_view).
```C++
const auto numbers = { 0, 1, 2, 3, 4, };
for (auto number : FilterRange(numbers, [](int number) { return number % 2 == 0; })) {
  // number will have the values 0, 2, 4
}
```

#### RangeAdapter
Equivalent of [`std::views::transform`](https://en.cppreference.com/w/cpp/ranges/transform_view).
```C++
const auto numbers = { 0, 1, 2, 3, 4, };
for (auto number : TransformRange(numbers, [](int number) { return number * 2; })) {
  // number will have the values 0, 2, 4, 6, 8
}
```

#### TupleElementRange
Equivalent of [`std::views::elements`](https://en.cppreference.com/w/cpp/ranges/elements_view).
```C++
const std::map<std::string, int> dict = { {"foo", 2}, {"bar", 4} };
for (auto key : Keys(dict)) {
  // "foo", "bar"
}
for (auto value : Values(dict)) {
  // 2, 4
}
```

## Error Handling
The header [result.hpp](utils/include/ovis/utils/result.hpp) contains a the `Result<T, E>` class which is used within the engine for error handling instead of error codes or exceptions.
`Result<T, E>` works similar to the proposed [`std::expected<T, E>`](https://en.cppreference.com/w/cpp/utility/expected) or [`Result<T, E>`](https://doc.rust-lang.org/std/result/) in Rust.
In addition, the header defines a simple `Error` which simply stores an error message.
This error type is sufficient for most use cases where you do not want to differentiate between different errors that can occur during a function call.
`Error` has a constructor that takes a format string and its corresponding parameters to produce the final message using [{fmt}](https://github.com/fmtlib/fmt) (look below for an example).

#### Usage
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

#### Usage of `void` Results
The Result class is specialized for `Result<void, E>`, which basically results in and `std::optional<E>`.
`Result<void, E>` does not have a has_value() method, but the `bool` operator is still overloaded to return true if the function succeeded and false if an error occured.
Similarly, it does not have the arrow and dereference operator overloaded, as this would not make sense in this context.
Constructing a successful `Result<void, E>` is also strange, as there is no `T` we can construct it with.
First, a default constructed `Result<void, E>` would indicate success, however, this was confusing so the `SuccessType` struct was introduced.
Similarly, to `std::nullopt_t` its only use is to construct a `Result<void, E>` that indicates success.
For convenience, the global declaration `constexpr SuccessType Success;` exists to allow simple construction of such types.
E.g.,:
```C++
Result<> WriteFile(const std::filesystem::path& path, std::string_view content) {
  std::ofstream file(path);

  if (!file) {
    return Error("Cannot open file: {}", path);
  }

  if (!file.write(content.data(), content.size())) {
    return Error("Could not write file: {}", path);
  }

  return Success;
}
```

#### Error propagation
When you want to propagate errors through a function call you can do it like this:
```C++
Result<Int> ReadIntegerFromFile(const std::filesystem::path& path) {
  const auto content = ReadFile(path);
  if (!content) {
    return content.error();
  }
  return std::stoi(*content); // Please use std::from_chars instead of stoi in real code!
}
```
However, the header also defined the `OVIS_CHECK_RESULT` macro which essentially does the same this as the if.
So, you can write this instead:
```C++
Result<Int> ReadIntegerFromFile(const std::filesystem::path& path) {
  const auto content = ReadFile(path);
  OVIS_CHECK_RESULT(content);
  return std::stoi(*content); // Again, use std::from_chars instead
}
```
Unfortunately, there is now way to implement a macro with similar functionality to the [`?` operator](https://doc.rust-lang.org/rust-by-example/std/result/question_mark.html) in Rust.

#### Safety
In debug mode, the Result class also checks whether the user perfomed a check if the result has a value before accessing it.
So, this would trigger an assertion:
```C++
std::string value = *ReadFile(some_filename); // Triggers an assertion in debug mode, even if the result contains a value
```
This is, to ensure that potential errors are not simply ignored.
It is not completely safe, as the following would pass, but it is better than nothing.
```C++
auto content = ReadFile(some_filename);
bool has_value = content.has_value(); // The return value of has_value() needs to be used as it is flagged with [[nodiscard]]
content->size(); // This would pass in debug mode, and would lead to undefined behaviour when content actually contains an error.
// I chose not to check for a value on every dereferencation due to runtime overhead (same as std::optional, ...).
```

#### Caveats
Currently there are two constructors of `Result<T, E>`: one which takes an T and one that takes an E.
This will lead to a compile-time error if T == E or if the constructor gets called with a type that is neither T or E but implicitly convertible to both.
However, these use cases are unlikely to occur in normal usage that I decided to not support this instead of wrapping the error like [`std::unexpected`](https://en.cppreference.com/w/cpp/utility/expected/unexpected).
