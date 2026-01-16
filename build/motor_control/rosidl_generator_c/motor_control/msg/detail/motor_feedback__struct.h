// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice

#ifndef MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__STRUCT_H_
#define MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'pos'
// Member 'vel'
// Member 'tor'
// Member 'temp'
// Member 'error_code'
// Member 'pattern'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/MotorFeedback in the package motor_control.
typedef struct motor_control__msg__MotorFeedback
{
  rosidl_runtime_c__float__Sequence pos;
  rosidl_runtime_c__float__Sequence vel;
  rosidl_runtime_c__float__Sequence tor;
  rosidl_runtime_c__float__Sequence temp;
  rosidl_runtime_c__uint8__Sequence error_code;
  rosidl_runtime_c__int8__Sequence pattern;
} motor_control__msg__MotorFeedback;

// Struct for a sequence of motor_control__msg__MotorFeedback.
typedef struct motor_control__msg__MotorFeedback__Sequence
{
  motor_control__msg__MotorFeedback * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} motor_control__msg__MotorFeedback__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MOTOR_CONTROL__MSG__DETAIL__MOTOR_FEEDBACK__STRUCT_H_
