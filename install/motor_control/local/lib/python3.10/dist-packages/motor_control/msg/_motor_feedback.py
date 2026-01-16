# generated from rosidl_generator_py/resource/_idl.py.em
# with input from motor_control:msg/MotorFeedback.idl
# generated code does not contain a copyright notice


# Import statements for member types

# Member 'pos'
# Member 'vel'
# Member 'tor'
# Member 'temp'
# Member 'error_code'
# Member 'pattern'
import array  # noqa: E402, I100

import builtins  # noqa: E402, I100

import math  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_MotorFeedback(type):
    """Metaclass of message 'MotorFeedback'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('motor_control')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'motor_control.msg.MotorFeedback')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__motor_feedback
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__motor_feedback
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__motor_feedback
            cls._TYPE_SUPPORT = module.type_support_msg__msg__motor_feedback
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__motor_feedback

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class MotorFeedback(metaclass=Metaclass_MotorFeedback):
    """Message class 'MotorFeedback'."""

    __slots__ = [
        '_pos',
        '_vel',
        '_tor',
        '_temp',
        '_error_code',
        '_pattern',
    ]

    _fields_and_field_types = {
        'pos': 'sequence<float>',
        'vel': 'sequence<float>',
        'tor': 'sequence<float>',
        'temp': 'sequence<float>',
        'error_code': 'sequence<uint8>',
        'pattern': 'sequence<int8>',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.UnboundedSequence(rosidl_parser.definition.BasicType('float')),  # noqa: E501
        rosidl_parser.definition.UnboundedSequence(rosidl_parser.definition.BasicType('float')),  # noqa: E501
        rosidl_parser.definition.UnboundedSequence(rosidl_parser.definition.BasicType('float')),  # noqa: E501
        rosidl_parser.definition.UnboundedSequence(rosidl_parser.definition.BasicType('float')),  # noqa: E501
        rosidl_parser.definition.UnboundedSequence(rosidl_parser.definition.BasicType('uint8')),  # noqa: E501
        rosidl_parser.definition.UnboundedSequence(rosidl_parser.definition.BasicType('int8')),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.pos = array.array('f', kwargs.get('pos', []))
        self.vel = array.array('f', kwargs.get('vel', []))
        self.tor = array.array('f', kwargs.get('tor', []))
        self.temp = array.array('f', kwargs.get('temp', []))
        self.error_code = array.array('B', kwargs.get('error_code', []))
        self.pattern = array.array('b', kwargs.get('pattern', []))

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.pos != other.pos:
            return False
        if self.vel != other.vel:
            return False
        if self.tor != other.tor:
            return False
        if self.temp != other.temp:
            return False
        if self.error_code != other.error_code:
            return False
        if self.pattern != other.pattern:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def pos(self):
        """Message field 'pos'."""
        return self._pos

    @pos.setter
    def pos(self, value):
        if isinstance(value, array.array):
            assert value.typecode == 'f', \
                "The 'pos' array.array() must have the type code of 'f'"
            self._pos = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 all(isinstance(v, float) for v in value) and
                 all(not (val < -3.402823466e+38 or val > 3.402823466e+38) or math.isinf(val) for val in value)), \
                "The 'pos' field must be a set or sequence and each value of type 'float' and each float in [-340282346600000016151267322115014000640.000000, 340282346600000016151267322115014000640.000000]"
        self._pos = array.array('f', value)

    @builtins.property
    def vel(self):
        """Message field 'vel'."""
        return self._vel

    @vel.setter
    def vel(self, value):
        if isinstance(value, array.array):
            assert value.typecode == 'f', \
                "The 'vel' array.array() must have the type code of 'f'"
            self._vel = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 all(isinstance(v, float) for v in value) and
                 all(not (val < -3.402823466e+38 or val > 3.402823466e+38) or math.isinf(val) for val in value)), \
                "The 'vel' field must be a set or sequence and each value of type 'float' and each float in [-340282346600000016151267322115014000640.000000, 340282346600000016151267322115014000640.000000]"
        self._vel = array.array('f', value)

    @builtins.property
    def tor(self):
        """Message field 'tor'."""
        return self._tor

    @tor.setter
    def tor(self, value):
        if isinstance(value, array.array):
            assert value.typecode == 'f', \
                "The 'tor' array.array() must have the type code of 'f'"
            self._tor = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 all(isinstance(v, float) for v in value) and
                 all(not (val < -3.402823466e+38 or val > 3.402823466e+38) or math.isinf(val) for val in value)), \
                "The 'tor' field must be a set or sequence and each value of type 'float' and each float in [-340282346600000016151267322115014000640.000000, 340282346600000016151267322115014000640.000000]"
        self._tor = array.array('f', value)

    @builtins.property
    def temp(self):
        """Message field 'temp'."""
        return self._temp

    @temp.setter
    def temp(self, value):
        if isinstance(value, array.array):
            assert value.typecode == 'f', \
                "The 'temp' array.array() must have the type code of 'f'"
            self._temp = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 all(isinstance(v, float) for v in value) and
                 all(not (val < -3.402823466e+38 or val > 3.402823466e+38) or math.isinf(val) for val in value)), \
                "The 'temp' field must be a set or sequence and each value of type 'float' and each float in [-340282346600000016151267322115014000640.000000, 340282346600000016151267322115014000640.000000]"
        self._temp = array.array('f', value)

    @builtins.property
    def error_code(self):
        """Message field 'error_code'."""
        return self._error_code

    @error_code.setter
    def error_code(self, value):
        if isinstance(value, array.array):
            assert value.typecode == 'B', \
                "The 'error_code' array.array() must have the type code of 'B'"
            self._error_code = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 all(isinstance(v, int) for v in value) and
                 all(val >= 0 and val < 256 for val in value)), \
                "The 'error_code' field must be a set or sequence and each value of type 'int' and each unsigned integer in [0, 255]"
        self._error_code = array.array('B', value)

    @builtins.property
    def pattern(self):
        """Message field 'pattern'."""
        return self._pattern

    @pattern.setter
    def pattern(self, value):
        if isinstance(value, array.array):
            assert value.typecode == 'b', \
                "The 'pattern' array.array() must have the type code of 'b'"
            self._pattern = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 all(isinstance(v, int) for v in value) and
                 all(val >= -128 and val < 128 for val in value)), \
                "The 'pattern' field must be a set or sequence and each value of type 'int' and each integer in [-128, 127]"
        self._pattern = array.array('b', value)
