// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from motor_control:msg/MotorFeedback.idl
// generated code does not contain a copyright notice
#include "motor_control/msg/detail/motor_feedback__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "motor_control/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "motor_control/msg/detail/motor_feedback__struct.h"
#include "motor_control/msg/detail/motor_feedback__functions.h"
#include "fastcdr/Cdr.h"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

// includes and forward declarations of message dependencies and their conversion functions

#if defined(__cplusplus)
extern "C"
{
#endif

#include "rosidl_runtime_c/primitives_sequence.h"  // error_code, pattern, pos, temp, tor, vel
#include "rosidl_runtime_c/primitives_sequence_functions.h"  // error_code, pattern, pos, temp, tor, vel

// forward declare type support functions


using _MotorFeedback__ros_msg_type = motor_control__msg__MotorFeedback;

static bool _MotorFeedback__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const _MotorFeedback__ros_msg_type * ros_message = static_cast<const _MotorFeedback__ros_msg_type *>(untyped_ros_message);
  // Field name: pos
  {
    size_t size = ros_message->pos.size;
    auto array_ptr = ros_message->pos.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: vel
  {
    size_t size = ros_message->vel.size;
    auto array_ptr = ros_message->vel.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: tor
  {
    size_t size = ros_message->tor.size;
    auto array_ptr = ros_message->tor.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: temp
  {
    size_t size = ros_message->temp.size;
    auto array_ptr = ros_message->temp.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: error_code
  {
    size_t size = ros_message->error_code.size;
    auto array_ptr = ros_message->error_code.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: pattern
  {
    size_t size = ros_message->pattern.size;
    auto array_ptr = ros_message->pattern.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  return true;
}

static bool _MotorFeedback__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  _MotorFeedback__ros_msg_type * ros_message = static_cast<_MotorFeedback__ros_msg_type *>(untyped_ros_message);
  // Field name: pos
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->pos.data) {
      rosidl_runtime_c__float__Sequence__fini(&ros_message->pos);
    }
    if (!rosidl_runtime_c__float__Sequence__init(&ros_message->pos, size)) {
      fprintf(stderr, "failed to create array for field 'pos'");
      return false;
    }
    auto array_ptr = ros_message->pos.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: vel
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->vel.data) {
      rosidl_runtime_c__float__Sequence__fini(&ros_message->vel);
    }
    if (!rosidl_runtime_c__float__Sequence__init(&ros_message->vel, size)) {
      fprintf(stderr, "failed to create array for field 'vel'");
      return false;
    }
    auto array_ptr = ros_message->vel.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: tor
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->tor.data) {
      rosidl_runtime_c__float__Sequence__fini(&ros_message->tor);
    }
    if (!rosidl_runtime_c__float__Sequence__init(&ros_message->tor, size)) {
      fprintf(stderr, "failed to create array for field 'tor'");
      return false;
    }
    auto array_ptr = ros_message->tor.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: temp
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->temp.data) {
      rosidl_runtime_c__float__Sequence__fini(&ros_message->temp);
    }
    if (!rosidl_runtime_c__float__Sequence__init(&ros_message->temp, size)) {
      fprintf(stderr, "failed to create array for field 'temp'");
      return false;
    }
    auto array_ptr = ros_message->temp.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: error_code
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->error_code.data) {
      rosidl_runtime_c__uint8__Sequence__fini(&ros_message->error_code);
    }
    if (!rosidl_runtime_c__uint8__Sequence__init(&ros_message->error_code, size)) {
      fprintf(stderr, "failed to create array for field 'error_code'");
      return false;
    }
    auto array_ptr = ros_message->error_code.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: pattern
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->pattern.data) {
      rosidl_runtime_c__int8__Sequence__fini(&ros_message->pattern);
    }
    if (!rosidl_runtime_c__int8__Sequence__init(&ros_message->pattern, size)) {
      fprintf(stderr, "failed to create array for field 'pattern'");
      return false;
    }
    auto array_ptr = ros_message->pattern.data;
    cdr.deserializeArray(array_ptr, size);
  }

  return true;
}  // NOLINT(readability/fn_size)

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_motor_control
size_t get_serialized_size_motor_control__msg__MotorFeedback(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _MotorFeedback__ros_msg_type * ros_message = static_cast<const _MotorFeedback__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // field.name pos
  {
    size_t array_size = ros_message->pos.size;
    auto array_ptr = ros_message->pos.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name vel
  {
    size_t array_size = ros_message->vel.size;
    auto array_ptr = ros_message->vel.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name tor
  {
    size_t array_size = ros_message->tor.size;
    auto array_ptr = ros_message->tor.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name temp
  {
    size_t array_size = ros_message->temp.size;
    auto array_ptr = ros_message->temp.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name error_code
  {
    size_t array_size = ros_message->error_code.size;
    auto array_ptr = ros_message->error_code.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name pattern
  {
    size_t array_size = ros_message->pattern.size;
    auto array_ptr = ros_message->pattern.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

static uint32_t _MotorFeedback__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_motor_control__msg__MotorFeedback(
      untyped_ros_message, 0));
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_motor_control
size_t max_serialized_size_motor_control__msg__MotorFeedback(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;

  // member: pos
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint32_t);
    current_alignment += array_size * sizeof(uint32_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
  }
  // member: vel
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint32_t);
    current_alignment += array_size * sizeof(uint32_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
  }
  // member: tor
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint32_t);
    current_alignment += array_size * sizeof(uint32_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
  }
  // member: temp
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint32_t);
    current_alignment += array_size * sizeof(uint32_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
  }
  // member: error_code
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: pattern
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = motor_control__msg__MotorFeedback;
    is_plain =
      (
      offsetof(DataType, pattern) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static size_t _MotorFeedback__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_motor_control__msg__MotorFeedback(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_MotorFeedback = {
  "motor_control::msg",
  "MotorFeedback",
  _MotorFeedback__cdr_serialize,
  _MotorFeedback__cdr_deserialize,
  _MotorFeedback__get_serialized_size,
  _MotorFeedback__max_serialized_size
};

static rosidl_message_type_support_t _MotorFeedback__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_MotorFeedback,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, motor_control, msg, MotorFeedback)() {
  return &_MotorFeedback__type_support;
}

#if defined(__cplusplus)
}
#endif
