// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "motor_control/msg/detail/motor_feedback__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace motor_control
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void MotorFeedback_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) motor_control::msg::MotorFeedback(_init);
}

void MotorFeedback_fini_function(void * message_memory)
{
  auto typed_message = static_cast<motor_control::msg::MotorFeedback *>(message_memory);
  typed_message->~MotorFeedback();
}

size_t size_function__MotorFeedback__pos(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MotorFeedback__pos(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__MotorFeedback__pos(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__MotorFeedback__pos(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__MotorFeedback__pos(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__MotorFeedback__pos(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__MotorFeedback__pos(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__MotorFeedback__pos(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__MotorFeedback__vel(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MotorFeedback__vel(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__MotorFeedback__vel(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__MotorFeedback__vel(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__MotorFeedback__vel(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__MotorFeedback__vel(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__MotorFeedback__vel(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__MotorFeedback__vel(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__MotorFeedback__tor(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MotorFeedback__tor(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__MotorFeedback__tor(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__MotorFeedback__tor(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__MotorFeedback__tor(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__MotorFeedback__tor(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__MotorFeedback__tor(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__MotorFeedback__tor(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__MotorFeedback__temp(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MotorFeedback__temp(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__MotorFeedback__temp(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__MotorFeedback__temp(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__MotorFeedback__temp(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__MotorFeedback__temp(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__MotorFeedback__temp(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__MotorFeedback__temp(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__MotorFeedback__error_code(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<uint8_t> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MotorFeedback__error_code(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<uint8_t> *>(untyped_member);
  return &member[index];
}

void * get_function__MotorFeedback__error_code(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<uint8_t> *>(untyped_member);
  return &member[index];
}

void fetch_function__MotorFeedback__error_code(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const uint8_t *>(
    get_const_function__MotorFeedback__error_code(untyped_member, index));
  auto & value = *reinterpret_cast<uint8_t *>(untyped_value);
  value = item;
}

void assign_function__MotorFeedback__error_code(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<uint8_t *>(
    get_function__MotorFeedback__error_code(untyped_member, index));
  const auto & value = *reinterpret_cast<const uint8_t *>(untyped_value);
  item = value;
}

void resize_function__MotorFeedback__error_code(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<uint8_t> *>(untyped_member);
  member->resize(size);
}

size_t size_function__MotorFeedback__pattern(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<int8_t> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MotorFeedback__pattern(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<int8_t> *>(untyped_member);
  return &member[index];
}

void * get_function__MotorFeedback__pattern(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<int8_t> *>(untyped_member);
  return &member[index];
}

void fetch_function__MotorFeedback__pattern(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const int8_t *>(
    get_const_function__MotorFeedback__pattern(untyped_member, index));
  auto & value = *reinterpret_cast<int8_t *>(untyped_value);
  value = item;
}

void assign_function__MotorFeedback__pattern(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<int8_t *>(
    get_function__MotorFeedback__pattern(untyped_member, index));
  const auto & value = *reinterpret_cast<const int8_t *>(untyped_value);
  item = value;
}

void resize_function__MotorFeedback__pattern(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<int8_t> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember MotorFeedback_message_member_array[6] = {
  {
    "pos",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control::msg::MotorFeedback, pos),  // bytes offset in struct
    nullptr,  // default value
    size_function__MotorFeedback__pos,  // size() function pointer
    get_const_function__MotorFeedback__pos,  // get_const(index) function pointer
    get_function__MotorFeedback__pos,  // get(index) function pointer
    fetch_function__MotorFeedback__pos,  // fetch(index, &value) function pointer
    assign_function__MotorFeedback__pos,  // assign(index, value) function pointer
    resize_function__MotorFeedback__pos  // resize(index) function pointer
  },
  {
    "vel",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control::msg::MotorFeedback, vel),  // bytes offset in struct
    nullptr,  // default value
    size_function__MotorFeedback__vel,  // size() function pointer
    get_const_function__MotorFeedback__vel,  // get_const(index) function pointer
    get_function__MotorFeedback__vel,  // get(index) function pointer
    fetch_function__MotorFeedback__vel,  // fetch(index, &value) function pointer
    assign_function__MotorFeedback__vel,  // assign(index, value) function pointer
    resize_function__MotorFeedback__vel  // resize(index) function pointer
  },
  {
    "tor",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control::msg::MotorFeedback, tor),  // bytes offset in struct
    nullptr,  // default value
    size_function__MotorFeedback__tor,  // size() function pointer
    get_const_function__MotorFeedback__tor,  // get_const(index) function pointer
    get_function__MotorFeedback__tor,  // get(index) function pointer
    fetch_function__MotorFeedback__tor,  // fetch(index, &value) function pointer
    assign_function__MotorFeedback__tor,  // assign(index, value) function pointer
    resize_function__MotorFeedback__tor  // resize(index) function pointer
  },
  {
    "temp",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control::msg::MotorFeedback, temp),  // bytes offset in struct
    nullptr,  // default value
    size_function__MotorFeedback__temp,  // size() function pointer
    get_const_function__MotorFeedback__temp,  // get_const(index) function pointer
    get_function__MotorFeedback__temp,  // get(index) function pointer
    fetch_function__MotorFeedback__temp,  // fetch(index, &value) function pointer
    assign_function__MotorFeedback__temp,  // assign(index, value) function pointer
    resize_function__MotorFeedback__temp  // resize(index) function pointer
  },
  {
    "error_code",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control::msg::MotorFeedback, error_code),  // bytes offset in struct
    nullptr,  // default value
    size_function__MotorFeedback__error_code,  // size() function pointer
    get_const_function__MotorFeedback__error_code,  // get_const(index) function pointer
    get_function__MotorFeedback__error_code,  // get(index) function pointer
    fetch_function__MotorFeedback__error_code,  // fetch(index, &value) function pointer
    assign_function__MotorFeedback__error_code,  // assign(index, value) function pointer
    resize_function__MotorFeedback__error_code  // resize(index) function pointer
  },
  {
    "pattern",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control::msg::MotorFeedback, pattern),  // bytes offset in struct
    nullptr,  // default value
    size_function__MotorFeedback__pattern,  // size() function pointer
    get_const_function__MotorFeedback__pattern,  // get_const(index) function pointer
    get_function__MotorFeedback__pattern,  // get(index) function pointer
    fetch_function__MotorFeedback__pattern,  // fetch(index, &value) function pointer
    assign_function__MotorFeedback__pattern,  // assign(index, value) function pointer
    resize_function__MotorFeedback__pattern  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers MotorFeedback_message_members = {
  "motor_control::msg",  // message namespace
  "MotorFeedback",  // message name
  6,  // number of fields
  sizeof(motor_control::msg::MotorFeedback),
  MotorFeedback_message_member_array,  // message members
  MotorFeedback_init_function,  // function to initialize message memory (memory has to be allocated)
  MotorFeedback_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t MotorFeedback_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &MotorFeedback_message_members,
  get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace motor_control


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<motor_control::msg::MotorFeedback>()
{
  return &::motor_control::msg::rosidl_typesupport_introspection_cpp::MotorFeedback_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, motor_control, msg, MotorFeedback)() {
  return &::motor_control::msg::rosidl_typesupport_introspection_cpp::MotorFeedback_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
