// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "motor_control/msg/detail/motor_feedback__rosidl_typesupport_introspection_c.h"
#include "motor_control/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "motor_control/msg/detail/motor_feedback__functions.h"
#include "motor_control/msg/detail/motor_feedback__struct.h"


// Include directives for member types
// Member `pos`
// Member `vel`
// Member `tor`
// Member `temp`
// Member `error_code`
// Member `pattern`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  motor_control__msg__MotorFeedback__init(message_memory);
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_fini_function(void * message_memory)
{
  motor_control__msg__MotorFeedback__fini(message_memory);
}

size_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__pos(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__pos(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__pos(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__pos(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__pos(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__pos(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__pos(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__pos(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__vel(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__vel(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__vel(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__vel(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__vel(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__vel(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__vel(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__vel(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__tor(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__tor(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__tor(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__tor(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__tor(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__tor(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__tor(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__tor(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__temp(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__temp(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__temp(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__temp(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__temp(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__temp(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__temp(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__temp(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__error_code(
  const void * untyped_member)
{
  const rosidl_runtime_c__uint8__Sequence * member =
    (const rosidl_runtime_c__uint8__Sequence *)(untyped_member);
  return member->size;
}

const void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__error_code(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__uint8__Sequence * member =
    (const rosidl_runtime_c__uint8__Sequence *)(untyped_member);
  return &member->data[index];
}

void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__error_code(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__uint8__Sequence * member =
    (rosidl_runtime_c__uint8__Sequence *)(untyped_member);
  return &member->data[index];
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__error_code(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const uint8_t * item =
    ((const uint8_t *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__error_code(untyped_member, index));
  uint8_t * value =
    (uint8_t *)(untyped_value);
  *value = *item;
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__error_code(
  void * untyped_member, size_t index, const void * untyped_value)
{
  uint8_t * item =
    ((uint8_t *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__error_code(untyped_member, index));
  const uint8_t * value =
    (const uint8_t *)(untyped_value);
  *item = *value;
}

bool motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__error_code(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__uint8__Sequence * member =
    (rosidl_runtime_c__uint8__Sequence *)(untyped_member);
  rosidl_runtime_c__uint8__Sequence__fini(member);
  return rosidl_runtime_c__uint8__Sequence__init(member, size);
}

size_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__pattern(
  const void * untyped_member)
{
  const rosidl_runtime_c__int8__Sequence * member =
    (const rosidl_runtime_c__int8__Sequence *)(untyped_member);
  return member->size;
}

const void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__pattern(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__int8__Sequence * member =
    (const rosidl_runtime_c__int8__Sequence *)(untyped_member);
  return &member->data[index];
}

void * motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__pattern(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__int8__Sequence * member =
    (rosidl_runtime_c__int8__Sequence *)(untyped_member);
  return &member->data[index];
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__pattern(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const int8_t * item =
    ((const int8_t *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__pattern(untyped_member, index));
  int8_t * value =
    (int8_t *)(untyped_value);
  *value = *item;
}

void motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__pattern(
  void * untyped_member, size_t index, const void * untyped_value)
{
  int8_t * item =
    ((int8_t *)
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__pattern(untyped_member, index));
  const int8_t * value =
    (const int8_t *)(untyped_value);
  *item = *value;
}

bool motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__pattern(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__int8__Sequence * member =
    (rosidl_runtime_c__int8__Sequence *)(untyped_member);
  rosidl_runtime_c__int8__Sequence__fini(member);
  return rosidl_runtime_c__int8__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_member_array[6] = {
  {
    "pos",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control__msg__MotorFeedback, pos),  // bytes offset in struct
    NULL,  // default value
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__pos,  // size() function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__pos,  // get_const(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__pos,  // get(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__pos,  // fetch(index, &value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__pos,  // assign(index, value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__pos  // resize(index) function pointer
  },
  {
    "vel",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control__msg__MotorFeedback, vel),  // bytes offset in struct
    NULL,  // default value
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__vel,  // size() function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__vel,  // get_const(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__vel,  // get(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__vel,  // fetch(index, &value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__vel,  // assign(index, value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__vel  // resize(index) function pointer
  },
  {
    "tor",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control__msg__MotorFeedback, tor),  // bytes offset in struct
    NULL,  // default value
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__tor,  // size() function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__tor,  // get_const(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__tor,  // get(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__tor,  // fetch(index, &value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__tor,  // assign(index, value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__tor  // resize(index) function pointer
  },
  {
    "temp",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control__msg__MotorFeedback, temp),  // bytes offset in struct
    NULL,  // default value
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__temp,  // size() function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__temp,  // get_const(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__temp,  // get(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__temp,  // fetch(index, &value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__temp,  // assign(index, value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__temp  // resize(index) function pointer
  },
  {
    "error_code",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control__msg__MotorFeedback, error_code),  // bytes offset in struct
    NULL,  // default value
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__error_code,  // size() function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__error_code,  // get_const(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__error_code,  // get(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__error_code,  // fetch(index, &value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__error_code,  // assign(index, value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__error_code  // resize(index) function pointer
  },
  {
    "pattern",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(motor_control__msg__MotorFeedback, pattern),  // bytes offset in struct
    NULL,  // default value
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__size_function__MotorFeedback__pattern,  // size() function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_const_function__MotorFeedback__pattern,  // get_const(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__get_function__MotorFeedback__pattern,  // get(index) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__fetch_function__MotorFeedback__pattern,  // fetch(index, &value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__assign_function__MotorFeedback__pattern,  // assign(index, value) function pointer
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__resize_function__MotorFeedback__pattern  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_members = {
  "motor_control__msg",  // message namespace
  "MotorFeedback",  // message name
  6,  // number of fields
  sizeof(motor_control__msg__MotorFeedback),
  motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_member_array,  // message members
  motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_init_function,  // function to initialize message memory (memory has to be allocated)
  motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_type_support_handle = {
  0,
  &motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_motor_control
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, motor_control, msg, MotorFeedback)() {
  if (!motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_type_support_handle.typesupport_identifier) {
    motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &motor_control__msg__MotorFeedback__rosidl_typesupport_introspection_c__MotorFeedback_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
