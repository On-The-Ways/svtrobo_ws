// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__STRUCT_HPP_
#define MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__motor_control__msg__MotorFeedback __attribute__((deprecated))
#else
# define DEPRECATED__motor_control__msg__MotorFeedback __declspec(deprecated)
#endif

namespace motor_control
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct MotorFeedback_
{
  using Type = MotorFeedback_<ContainerAllocator>;

  explicit MotorFeedback_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit MotorFeedback_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _pos_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _pos_type pos;
  using _vel_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _vel_type vel;
  using _tor_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _tor_type tor;
  using _temp_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _temp_type temp;
  using _error_code_type =
    std::vector<uint8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<uint8_t>>;
  _error_code_type error_code;
  using _pattern_type =
    std::vector<int8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<int8_t>>;
  _pattern_type pattern;

  // setters for named parameter idiom
  Type & set__pos(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->pos = _arg;
    return *this;
  }
  Type & set__vel(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->vel = _arg;
    return *this;
  }
  Type & set__tor(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->tor = _arg;
    return *this;
  }
  Type & set__temp(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->temp = _arg;
    return *this;
  }
  Type & set__error_code(
    const std::vector<uint8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<uint8_t>> & _arg)
  {
    this->error_code = _arg;
    return *this;
  }
  Type & set__pattern(
    const std::vector<int8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<int8_t>> & _arg)
  {
    this->pattern = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    motor_control::msg::MotorFeedback_<ContainerAllocator> *;
  using ConstRawPtr =
    const motor_control::msg::MotorFeedback_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      motor_control::msg::MotorFeedback_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      motor_control::msg::MotorFeedback_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__motor_control__msg__MotorFeedback
    std::shared_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__motor_control__msg__MotorFeedback
    std::shared_ptr<motor_control::msg::MotorFeedback_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const MotorFeedback_ & other) const
  {
    if (this->pos != other.pos) {
      return false;
    }
    if (this->vel != other.vel) {
      return false;
    }
    if (this->tor != other.tor) {
      return false;
    }
    if (this->temp != other.temp) {
      return false;
    }
    if (this->error_code != other.error_code) {
      return false;
    }
    if (this->pattern != other.pattern) {
      return false;
    }
    return true;
  }
  bool operator!=(const MotorFeedback_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct MotorFeedback_

// alias to use template instance with default allocator
using MotorFeedback =
  motor_control::msg::MotorFeedback_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace motor_control

#endif  // MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__STRUCT_HPP_
