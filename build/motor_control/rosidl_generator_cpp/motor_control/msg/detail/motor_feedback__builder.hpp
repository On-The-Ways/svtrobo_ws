// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__BUILDER_HPP_
#define MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "motor_control/msg/detail/motor_feedback__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace motor_control
{

namespace msg
{

namespace builder
{

class Init_MotorFeedback_pattern
{
public:
  explicit Init_MotorFeedback_pattern(::motor_control::msg::MotorFeedback & msg)
  : msg_(msg)
  {}
  ::motor_control::msg::MotorFeedback pattern(::motor_control::msg::MotorFeedback::_pattern_type arg)
  {
    msg_.pattern = std::move(arg);
    return std::move(msg_);
  }

private:
  ::motor_control::msg::MotorFeedback msg_;
};

class Init_MotorFeedback_error_code
{
public:
  explicit Init_MotorFeedback_error_code(::motor_control::msg::MotorFeedback & msg)
  : msg_(msg)
  {}
  Init_MotorFeedback_pattern error_code(::motor_control::msg::MotorFeedback::_error_code_type arg)
  {
    msg_.error_code = std::move(arg);
    return Init_MotorFeedback_pattern(msg_);
  }

private:
  ::motor_control::msg::MotorFeedback msg_;
};

class Init_MotorFeedback_temp
{
public:
  explicit Init_MotorFeedback_temp(::motor_control::msg::MotorFeedback & msg)
  : msg_(msg)
  {}
  Init_MotorFeedback_error_code temp(::motor_control::msg::MotorFeedback::_temp_type arg)
  {
    msg_.temp = std::move(arg);
    return Init_MotorFeedback_error_code(msg_);
  }

private:
  ::motor_control::msg::MotorFeedback msg_;
};

class Init_MotorFeedback_tor
{
public:
  explicit Init_MotorFeedback_tor(::motor_control::msg::MotorFeedback & msg)
  : msg_(msg)
  {}
  Init_MotorFeedback_temp tor(::motor_control::msg::MotorFeedback::_tor_type arg)
  {
    msg_.tor = std::move(arg);
    return Init_MotorFeedback_temp(msg_);
  }

private:
  ::motor_control::msg::MotorFeedback msg_;
};

class Init_MotorFeedback_vel
{
public:
  explicit Init_MotorFeedback_vel(::motor_control::msg::MotorFeedback & msg)
  : msg_(msg)
  {}
  Init_MotorFeedback_tor vel(::motor_control::msg::MotorFeedback::_vel_type arg)
  {
    msg_.vel = std::move(arg);
    return Init_MotorFeedback_tor(msg_);
  }

private:
  ::motor_control::msg::MotorFeedback msg_;
};

class Init_MotorFeedback_pos
{
public:
  Init_MotorFeedback_pos()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_MotorFeedback_vel pos(::motor_control::msg::MotorFeedback::_pos_type arg)
  {
    msg_.pos = std::move(arg);
    return Init_MotorFeedback_vel(msg_);
  }

private:
  ::motor_control::msg::MotorFeedback msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::motor_control::msg::MotorFeedback>()
{
  return motor_control::msg::builder::Init_MotorFeedback_pos();
}

}  // namespace motor_control

#endif  // MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__BUILDER_HPP_
