// Generated by the protocol buffer compiler.  DO NOT EDIT!

#ifndef PROTOBUF_MessageFormat_2eproto__INCLUDED
#define PROTOBUF_MessageFormat_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2002000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2002000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>

namespace AANF_Message {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_MessageFormat_2eproto();
void protobuf_AssignDesc_MessageFormat_2eproto();
void protobuf_ShutdownFile_MessageFormat_2eproto();

class MessageFormat;

// ===================================================================

class MessageFormat : public ::google::protobuf::Message {
 public:
  MessageFormat();
  virtual ~MessageFormat();
  
  MessageFormat(const MessageFormat& from);
  
  inline MessageFormat& operator=(const MessageFormat& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const MessageFormat& default_instance();
  void Swap(MessageFormat* other);
  
  // implements Message ----------------------------------------------
  
  MessageFormat* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const MessageFormat& from);
  void MergeFrom(const MessageFormat& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const { _cached_size_ = size; }
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required fixed32 length = 1;
  inline bool has_length() const;
  inline void clear_length();
  static const int kLengthFieldNumber = 1;
  inline ::google::protobuf::uint32 length() const;
  inline void set_length(::google::protobuf::uint32 value);
  
  // required int32 service_id = 11;
  inline bool has_service_id() const;
  inline void clear_service_id();
  static const int kServiceIdFieldNumber = 11;
  inline ::google::protobuf::int32 service_id() const;
  inline void set_service_id(::google::protobuf::int32 value);
  
  // required int32 type = 12;
  inline bool has_type() const;
  inline void clear_type();
  static const int kTypeFieldNumber = 12;
  inline ::google::protobuf::int32 type() const;
  inline void set_type(::google::protobuf::int32 value);
  
  // required int32 version = 13;
  inline bool has_version() const;
  inline void clear_version();
  static const int kVersionFieldNumber = 13;
  inline ::google::protobuf::int32 version() const;
  inline void set_version(::google::protobuf::int32 value);
  
  // required int64 seq = 14;
  inline bool has_seq() const;
  inline void clear_seq();
  static const int kSeqFieldNumber = 14;
  inline ::google::protobuf::int64 seq() const;
  inline void set_seq(::google::protobuf::int64 value);
  
  GOOGLE_PROTOBUF_EXTENSION_ACCESSORS(MessageFormat)
 private:
  ::google::protobuf::internal::ExtensionSet _extensions_;
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  mutable int _cached_size_;
  
  ::google::protobuf::uint32 length_;
  ::google::protobuf::int32 service_id_;
  ::google::protobuf::int32 type_;
  ::google::protobuf::int32 version_;
  ::google::protobuf::int64 seq_;
  friend void  protobuf_AddDesc_MessageFormat_2eproto();
  friend void protobuf_AssignDesc_MessageFormat_2eproto();
  friend void protobuf_ShutdownFile_MessageFormat_2eproto();
  
  ::google::protobuf::uint32 _has_bits_[(5 + 31) / 32];
  
  // WHY DOES & HAVE LOWER PRECEDENCE THAN != !?
  inline bool _has_bit(int index) const {
    return (_has_bits_[index / 32] & (1u << (index % 32))) != 0;
  }
  inline void _set_bit(int index) {
    _has_bits_[index / 32] |= (1u << (index % 32));
  }
  inline void _clear_bit(int index) {
    _has_bits_[index / 32] &= ~(1u << (index % 32));
  }
  
  void InitAsDefaultInstance();
  static MessageFormat* default_instance_;
};
// ===================================================================


// ===================================================================


// ===================================================================

// MessageFormat

// required fixed32 length = 1;
inline bool MessageFormat::has_length() const {
  return _has_bit(0);
}
inline void MessageFormat::clear_length() {
  length_ = 0u;
  _clear_bit(0);
}
inline ::google::protobuf::uint32 MessageFormat::length() const {
  return length_;
}
inline void MessageFormat::set_length(::google::protobuf::uint32 value) {
  _set_bit(0);
  length_ = value;
}

// required int32 service_id = 11;
inline bool MessageFormat::has_service_id() const {
  return _has_bit(1);
}
inline void MessageFormat::clear_service_id() {
  service_id_ = 0;
  _clear_bit(1);
}
inline ::google::protobuf::int32 MessageFormat::service_id() const {
  return service_id_;
}
inline void MessageFormat::set_service_id(::google::protobuf::int32 value) {
  _set_bit(1);
  service_id_ = value;
}

// required int32 type = 12;
inline bool MessageFormat::has_type() const {
  return _has_bit(2);
}
inline void MessageFormat::clear_type() {
  type_ = 0;
  _clear_bit(2);
}
inline ::google::protobuf::int32 MessageFormat::type() const {
  return type_;
}
inline void MessageFormat::set_type(::google::protobuf::int32 value) {
  _set_bit(2);
  type_ = value;
}

// required int32 version = 13;
inline bool MessageFormat::has_version() const {
  return _has_bit(3);
}
inline void MessageFormat::clear_version() {
  version_ = 0;
  _clear_bit(3);
}
inline ::google::protobuf::int32 MessageFormat::version() const {
  return version_;
}
inline void MessageFormat::set_version(::google::protobuf::int32 value) {
  _set_bit(3);
  version_ = value;
}

// required int64 seq = 14;
inline bool MessageFormat::has_seq() const {
  return _has_bit(4);
}
inline void MessageFormat::clear_seq() {
  seq_ = GOOGLE_LONGLONG(0);
  _clear_bit(4);
}
inline ::google::protobuf::int64 MessageFormat::seq() const {
  return seq_;
}
inline void MessageFormat::set_seq(::google::protobuf::int64 value) {
  _set_bit(4);
  seq_ = value;
}


}  // namespace AANF_Message

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

#endif  // PROTOBUF_MessageFormat_2eproto__INCLUDED
