// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice
#include "motor_control/msg/detail/motor_feedback__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `pos`
// Member `vel`
// Member `tor`
// Member `temp`
// Member `error_code`
// Member `pattern`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
motor_control__msg__MotorFeedback__init(motor_control__msg__MotorFeedback * msg)
{
  if (!msg) {
    return false;
  }
  // pos
  if (!rosidl_runtime_c__float__Sequence__init(&msg->pos, 0)) {
    motor_control__msg__MotorFeedback__fini(msg);
    return false;
  }
  // vel
  if (!rosidl_runtime_c__float__Sequence__init(&msg->vel, 0)) {
    motor_control__msg__MotorFeedback__fini(msg);
    return false;
  }
  // tor
  if (!rosidl_runtime_c__float__Sequence__init(&msg->tor, 0)) {
    motor_control__msg__MotorFeedback__fini(msg);
    return false;
  }
  // temp
  if (!rosidl_runtime_c__float__Sequence__init(&msg->temp, 0)) {
    motor_control__msg__MotorFeedback__fini(msg);
    return false;
  }
  // error_code
  if (!rosidl_runtime_c__uint8__Sequence__init(&msg->error_code, 0)) {
    motor_control__msg__MotorFeedback__fini(msg);
    return false;
  }
  // pattern
  if (!rosidl_runtime_c__int8__Sequence__init(&msg->pattern, 0)) {
    motor_control__msg__MotorFeedback__fini(msg);
    return false;
  }
  return true;
}

void
motor_control__msg__MotorFeedback__fini(motor_control__msg__MotorFeedback * msg)
{
  if (!msg) {
    return;
  }
  // pos
  rosidl_runtime_c__float__Sequence__fini(&msg->pos);
  // vel
  rosidl_runtime_c__float__Sequence__fini(&msg->vel);
  // tor
  rosidl_runtime_c__float__Sequence__fini(&msg->tor);
  // temp
  rosidl_runtime_c__float__Sequence__fini(&msg->temp);
  // error_code
  rosidl_runtime_c__uint8__Sequence__fini(&msg->error_code);
  // pattern
  rosidl_runtime_c__int8__Sequence__fini(&msg->pattern);
}

bool
motor_control__msg__MotorFeedback__are_equal(const motor_control__msg__MotorFeedback * lhs, const motor_control__msg__MotorFeedback * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // pos
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->pos), &(rhs->pos)))
  {
    return false;
  }
  // vel
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->vel), &(rhs->vel)))
  {
    return false;
  }
  // tor
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->tor), &(rhs->tor)))
  {
    return false;
  }
  // temp
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->temp), &(rhs->temp)))
  {
    return false;
  }
  // error_code
  if (!rosidl_runtime_c__uint8__Sequence__are_equal(
      &(lhs->error_code), &(rhs->error_code)))
  {
    return false;
  }
  // pattern
  if (!rosidl_runtime_c__int8__Sequence__are_equal(
      &(lhs->pattern), &(rhs->pattern)))
  {
    return false;
  }
  return true;
}

bool
motor_control__msg__MotorFeedback__copy(
  const motor_control__msg__MotorFeedback * input,
  motor_control__msg__MotorFeedback * output)
{
  if (!input || !output) {
    return false;
  }
  // pos
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->pos), &(output->pos)))
  {
    return false;
  }
  // vel
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->vel), &(output->vel)))
  {
    return false;
  }
  // tor
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->tor), &(output->tor)))
  {
    return false;
  }
  // temp
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->temp), &(output->temp)))
  {
    return false;
  }
  // error_code
  if (!rosidl_runtime_c__uint8__Sequence__copy(
      &(input->error_code), &(output->error_code)))
  {
    return false;
  }
  // pattern
  if (!rosidl_runtime_c__int8__Sequence__copy(
      &(input->pattern), &(output->pattern)))
  {
    return false;
  }
  return true;
}

motor_control__msg__MotorFeedback *
motor_control__msg__MotorFeedback__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  motor_control__msg__MotorFeedback * msg = (motor_control__msg__MotorFeedback *)allocator.allocate(sizeof(motor_control__msg__MotorFeedback), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(motor_control__msg__MotorFeedback));
  bool success = motor_control__msg__MotorFeedback__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
motor_control__msg__MotorFeedback__destroy(motor_control__msg__MotorFeedback * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    motor_control__msg__MotorFeedback__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
motor_control__msg__MotorFeedback__Sequence__init(motor_control__msg__MotorFeedback__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  motor_control__msg__MotorFeedback * data = NULL;

  if (size) {
    data = (motor_control__msg__MotorFeedback *)allocator.zero_allocate(size, sizeof(motor_control__msg__MotorFeedback), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = motor_control__msg__MotorFeedback__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        motor_control__msg__MotorFeedback__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
motor_control__msg__MotorFeedback__Sequence__fini(motor_control__msg__MotorFeedback__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      motor_control__msg__MotorFeedback__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

motor_control__msg__MotorFeedback__Sequence *
motor_control__msg__MotorFeedback__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  motor_control__msg__MotorFeedback__Sequence * array = (motor_control__msg__MotorFeedback__Sequence *)allocator.allocate(sizeof(motor_control__msg__MotorFeedback__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = motor_control__msg__MotorFeedback__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
motor_control__msg__MotorFeedback__Sequence__destroy(motor_control__msg__MotorFeedback__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    motor_control__msg__MotorFeedback__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
motor_control__msg__MotorFeedback__Sequence__are_equal(const motor_control__msg__MotorFeedback__Sequence * lhs, const motor_control__msg__MotorFeedback__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!motor_control__msg__MotorFeedback__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
motor_control__msg__MotorFeedback__Sequence__copy(
  const motor_control__msg__MotorFeedback__Sequence * input,
  motor_control__msg__MotorFeedback__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(motor_control__msg__MotorFeedback);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    motor_control__msg__MotorFeedback * data =
      (motor_control__msg__MotorFeedback *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!motor_control__msg__MotorFeedback__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          motor_control__msg__MotorFeedback__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!motor_control__msg__MotorFeedback__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
