// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__TRAITS_HPP_
#define MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "motor_control/msg/detail/motor_feedback__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace motor_control
{

namespace msg
{

inline void to_flow_style_yaml(
  const MotorFeedback & msg,
  std::ostream & out)
{
  out << "{";
  // member: pos
  {
    if (msg.pos.size() == 0) {
      out << "pos: []";
    } else {
      out << "pos: [";
      size_t pending_items = msg.pos.size();
      for (auto item : msg.pos) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: vel
  {
    if (msg.vel.size() == 0) {
      out << "vel: []";
    } else {
      out << "vel: [";
      size_t pending_items = msg.vel.size();
      for (auto item : msg.vel) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: tor
  {
    if (msg.tor.size() == 0) {
      out << "tor: []";
    } else {
      out << "tor: [";
      size_t pending_items = msg.tor.size();
      for (auto item : msg.tor) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: temp
  {
    if (msg.temp.size() == 0) {
      out << "temp: []";
    } else {
      out << "temp: [";
      size_t pending_items = msg.temp.size();
      for (auto item : msg.temp) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: error_code
  {
    if (msg.error_code.size() == 0) {
      out << "error_code: []";
    } else {
      out << "error_code: [";
      size_t pending_items = msg.error_code.size();
      for (auto item : msg.error_code) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: pattern
  {
    if (msg.pattern.size() == 0) {
      out << "pattern: []";
    } else {
      out << "pattern: [";
      size_t pending_items = msg.pattern.size();
      for (auto item : msg.pattern) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const MotorFeedback & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: pos
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.pos.size() == 0) {
      out << "pos: []\n";
    } else {
      out << "pos:\n";
      for (auto item : msg.pos) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: vel
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.vel.size() == 0) {
      out << "vel: []\n";
    } else {
      out << "vel:\n";
      for (auto item : msg.vel) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: tor
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.tor.size() == 0) {
      out << "tor: []\n";
    } else {
      out << "tor:\n";
      for (auto item : msg.tor) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: temp
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.temp.size() == 0) {
      out << "temp: []\n";
    } else {
      out << "temp:\n";
      for (auto item : msg.temp) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: error_code
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.error_code.size() == 0) {
      out << "error_code: []\n";
    } else {
      out << "error_code:\n";
      for (auto item : msg.error_code) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: pattern
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.pattern.size() == 0) {
      out << "pattern: []\n";
    } else {
      out << "pattern:\n";
      for (auto item : msg.pattern) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const MotorFeedback & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace motor_control

namespace rosidl_generator_traits
{

[[deprecated("use motor_control::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const motor_control::msg::MotorFeedback & msg,
  std::ostream & out, size_t indentation = 0)
{
  motor_control::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use motor_control::msg::to_yaml() instead")]]
inline std::string to_yaml(const motor_control::msg::MotorFeedback & msg)
{
  return motor_control::msg::to_yaml(msg);
}

template<>
inline const char * data_type<motor_control::msg::MotorFeedback>()
{
  return "motor_control::msg::MotorFeedback";
}

template<>
inline const char * name<motor_control::msg::MotorFeedback>()
{
  return "motor_control/msg/MotorFeedback";
}

template<>
struct has_fixed_size<motor_control::msg::MotorFeedback>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<motor_control::msg::MotorFeedback>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<motor_control::msg::MotorFeedback>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__TRAITS_HPP_
