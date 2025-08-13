#pragma once

#include <cassert>
#include <string_view>

namespace itst {
// This function is copied from LLVM and slightly changed (replaced StringRef by
// std::string_view)
template <typename DesiredTypeName>
constexpr std::string_view getTypeName() noexcept {
#if defined(__clang__) || defined(__GNUC__)
  std::string_view Name = __PRETTY_FUNCTION__;

  std::string_view Key = "DesiredTypeName = ";
  Name = Name.substr(Name.find(Key));
  assert(!Name.empty() && "Unable to find the template parameter!");
  Name = Name.substr(Key.size());

  assert(!Name.empty() && Name.back() == ']' &&
         "Name doesn't end in the substitution key!");
  return Name.substr(0, Name.size() - 1);
#elif defined(_MSC_VER)
  std::string_view Name = __FUNCSIG__;

  std::string_view Key = "getTypeName<";
  Name = Name.substr(Name.find(Key));
  assert(!Name.empty() && "Unable to find the function name!");
  Name = Name.substr(Key.size());

  for (std::string_view Prefix : {"class ", "struct ", "union ", "enum "})
    if (Name.size() >= Prefix.size() &&
        Name.substr(0, Prefix.size()) == Prefix) {
      Name = Name.substr(Prefix.size());
      break;
    }

  auto AnglePos = Name.rfind('>');
  assert(AnglePos != std::string_view::npos &&
         "Unable to find the closing '>'!");
  return Name.substr(0, AnglePos);
#else
  // No known technique for statically extracting a type name on this compiler.
  // We return a string that is unlikely to look like any type in LLVM.
  return "UNKNOWN_TYPE";
#endif
}
} // namespace itst
