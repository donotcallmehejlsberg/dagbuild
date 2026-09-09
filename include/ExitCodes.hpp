#ifndef EXIT_CODES_HPP
#define EXIT_CODES_HPP

namespace ExitCode {

inline constexpr int SUCCESS = 0;
inline constexpr int INVALID_COMMAND = 1;
inline constexpr int CONFIGURATION_ERROR = 2;
inline constexpr int BUILD_ERROR = 3;

}  // namespace ExitCode

#endif  // EXIT_CODES_HPP