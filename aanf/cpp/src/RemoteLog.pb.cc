// Generated by the protocol buffer compiler.  DO NOT EDIT!

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "RemoteLog.pb.h"
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>

namespace AANF_Protocol {

namespace {

const ::google::protobuf::Descriptor* RemoteLogRequest_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  RemoteLogRequest_reflection_ = NULL;
const ::google::protobuf::Descriptor* RemoteLogResponse_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  RemoteLogResponse_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* RemoteLogResponse_ErrorCode_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_RemoteLog_2eproto() {
  protobuf_AddDesc_RemoteLog_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "RemoteLog.proto");
  GOOGLE_CHECK(file != NULL);
  RemoteLogRequest_descriptor_ = file->message_type(0);
  static const int RemoteLogRequest_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RemoteLogRequest, log_data_),
  };
  RemoteLogRequest_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      RemoteLogRequest_descriptor_,
      RemoteLogRequest::default_instance_,
      RemoteLogRequest_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RemoteLogRequest, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RemoteLogRequest, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(RemoteLogRequest));
  RemoteLogResponse_descriptor_ = file->message_type(1);
  static const int RemoteLogResponse_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RemoteLogResponse, error_),
  };
  RemoteLogResponse_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      RemoteLogResponse_descriptor_,
      RemoteLogResponse::default_instance_,
      RemoteLogResponse_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RemoteLogResponse, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RemoteLogResponse, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(RemoteLogResponse));
  RemoteLogResponse_ErrorCode_descriptor_ = RemoteLogResponse_descriptor_->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_RemoteLog_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    RemoteLogRequest_descriptor_, &RemoteLogRequest::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    RemoteLogResponse_descriptor_, &RemoteLogResponse::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_RemoteLog_2eproto() {
  delete RemoteLogRequest::default_instance_;
  delete RemoteLogRequest_reflection_;
  delete RemoteLogResponse::default_instance_;
  delete RemoteLogResponse_reflection_;
}

void protobuf_AddDesc_RemoteLog_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::Protocol::protobuf_AddDesc_PacketFormat_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\017RemoteLog.proto\022\rAANF_Protocol\032\022Packet"
    "Format.proto\"$\n\020RemoteLogRequest\022\020\n\010log_"
    "data\030\004 \002(\t\"w\n\021RemoteLogResponse\0229\n\005error"
    "\030\004 \002(\0162*.AANF_Protocol.RemoteLogResponse"
    ".ErrorCode\"\'\n\tErrorCode\022\006\n\002OK\020\000\022\022\n\016NO_SU"
    "CH_LOG_ID\020\001:O\n\016remote_log_req\022\026.Protocol"
    ".PacketFormat\030e \001(\0132\037.AANF_Protocol.Remo"
    "teLogRequest:P\n\016remote_log_rsp\022\026.Protoco"
    "l.PacketFormat\030f \001(\0132 .AANF_Protocol.Rem"
    "oteLogResponse", 374);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "RemoteLog.proto", &protobuf_RegisterTypes);
  RemoteLogRequest::default_instance_ = new RemoteLogRequest();
  RemoteLogResponse::default_instance_ = new RemoteLogResponse();
  ::google::protobuf::internal::ExtensionSet::RegisterMessageExtension(
    &::Protocol::PacketFormat::default_instance(),
    101, 11, false, false,
    &::AANF_Protocol::RemoteLogRequest::default_instance());
  ::google::protobuf::internal::ExtensionSet::RegisterMessageExtension(
    &::Protocol::PacketFormat::default_instance(),
    102, 11, false, false,
    &::AANF_Protocol::RemoteLogResponse::default_instance());
  RemoteLogRequest::default_instance_->InitAsDefaultInstance();
  RemoteLogResponse::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_RemoteLog_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_RemoteLog_2eproto {
  StaticDescriptorInitializer_RemoteLog_2eproto() {
    protobuf_AddDesc_RemoteLog_2eproto();
  }
} static_descriptor_initializer_RemoteLog_2eproto_;


// ===================================================================

const ::std::string RemoteLogRequest::_default_log_data_;
#ifndef _MSC_VER
const int RemoteLogRequest::kLogDataFieldNumber;
#endif  // !_MSC_VER

RemoteLogRequest::RemoteLogRequest() {
  SharedCtor();
}

void RemoteLogRequest::InitAsDefaultInstance() {
}

RemoteLogRequest::RemoteLogRequest(const RemoteLogRequest& from) {
  SharedCtor();
  MergeFrom(from);
}

void RemoteLogRequest::SharedCtor() {
  _cached_size_ = 0;
  log_data_ = const_cast< ::std::string*>(&_default_log_data_);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

RemoteLogRequest::~RemoteLogRequest() {
  SharedDtor();
}

void RemoteLogRequest::SharedDtor() {
  if (log_data_ != &_default_log_data_) {
    delete log_data_;
  }
  if (this != default_instance_) {
  }
}

const ::google::protobuf::Descriptor* RemoteLogRequest::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return RemoteLogRequest_descriptor_;
}

const RemoteLogRequest& RemoteLogRequest::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_RemoteLog_2eproto();  return *default_instance_;
}

RemoteLogRequest* RemoteLogRequest::default_instance_ = NULL;

RemoteLogRequest* RemoteLogRequest::New() const {
  return new RemoteLogRequest;
}

void RemoteLogRequest::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (_has_bit(0)) {
      if (log_data_ != &_default_log_data_) {
        log_data_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool RemoteLogRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string log_data = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          goto handle_uninterpreted;
        }
        DO_(::google::protobuf::internal::WireFormatLite::ReadString(
              input, this->mutable_log_data()));
        ::google::protobuf::internal::WireFormat::VerifyUTF8String(
          this->log_data().data(), this->log_data().length(),
          ::google::protobuf::internal::WireFormat::PARSE);
        if (input->ExpectAtEnd()) return true;
        break;
      }
      
      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void RemoteLogRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  ::google::protobuf::uint8* raw_buffer = output->GetDirectBufferForNBytesAndAdvance(_cached_size_);
  if (raw_buffer != NULL) {
    RemoteLogRequest::SerializeWithCachedSizesToArray(raw_buffer);
    return;
  }
  
  // required string log_data = 4;
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->log_data().data(), this->log_data().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      4, this->log_data(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* RemoteLogRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required string log_data = 4;
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->log_data().data(), this->log_data().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->log_data(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int RemoteLogRequest::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required string log_data = 4;
    if (has_log_data()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->log_data());
    }
    
  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  _cached_size_ = total_size;
  return total_size;
}

void RemoteLogRequest::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const RemoteLogRequest* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const RemoteLogRequest*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void RemoteLogRequest::MergeFrom(const RemoteLogRequest& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from._has_bit(0)) {
      set_log_data(from.log_data());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void RemoteLogRequest::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void RemoteLogRequest::CopyFrom(const RemoteLogRequest& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool RemoteLogRequest::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;
  
  return true;
}

void RemoteLogRequest::Swap(RemoteLogRequest* other) {
  if (other != this) {
    std::swap(log_data_, other->log_data_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata RemoteLogRequest::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = RemoteLogRequest_descriptor_;
  metadata.reflection = RemoteLogRequest_reflection_;
  return metadata;
}


// ===================================================================

const ::google::protobuf::EnumDescriptor* RemoteLogResponse_ErrorCode_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return RemoteLogResponse_ErrorCode_descriptor_;
}
bool RemoteLogResponse_ErrorCode_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const RemoteLogResponse_ErrorCode RemoteLogResponse::OK;
const RemoteLogResponse_ErrorCode RemoteLogResponse::NO_SUCH_LOG_ID;
const RemoteLogResponse_ErrorCode RemoteLogResponse::ErrorCode_MIN;
const RemoteLogResponse_ErrorCode RemoteLogResponse::ErrorCode_MAX;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int RemoteLogResponse::kErrorFieldNumber;
#endif  // !_MSC_VER

RemoteLogResponse::RemoteLogResponse() {
  SharedCtor();
}

void RemoteLogResponse::InitAsDefaultInstance() {
}

RemoteLogResponse::RemoteLogResponse(const RemoteLogResponse& from) {
  SharedCtor();
  MergeFrom(from);
}

void RemoteLogResponse::SharedCtor() {
  _cached_size_ = 0;
  error_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

RemoteLogResponse::~RemoteLogResponse() {
  SharedDtor();
}

void RemoteLogResponse::SharedDtor() {
  if (this != default_instance_) {
  }
}

const ::google::protobuf::Descriptor* RemoteLogResponse::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return RemoteLogResponse_descriptor_;
}

const RemoteLogResponse& RemoteLogResponse::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_RemoteLog_2eproto();  return *default_instance_;
}

RemoteLogResponse* RemoteLogResponse::default_instance_ = NULL;

RemoteLogResponse* RemoteLogResponse::New() const {
  return new RemoteLogResponse;
}

void RemoteLogResponse::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    error_ = 0;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool RemoteLogResponse::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .AANF_Protocol.RemoteLogResponse.ErrorCode error = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
        int value;
        DO_(::google::protobuf::internal::WireFormatLite::ReadEnum(input, &value));
        if (::AANF_Protocol::RemoteLogResponse_ErrorCode_IsValid(value)) {
          set_error(static_cast< ::AANF_Protocol::RemoteLogResponse_ErrorCode >(value));
        } else {
          mutable_unknown_fields()->AddVarint(4, value);
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }
      
      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void RemoteLogResponse::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  ::google::protobuf::uint8* raw_buffer = output->GetDirectBufferForNBytesAndAdvance(_cached_size_);
  if (raw_buffer != NULL) {
    RemoteLogResponse::SerializeWithCachedSizesToArray(raw_buffer);
    return;
  }
  
  // required .AANF_Protocol.RemoteLogResponse.ErrorCode error = 4;
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      4, this->error(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* RemoteLogResponse::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required .AANF_Protocol.RemoteLogResponse.ErrorCode error = 4;
  if (_has_bit(0)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      4, this->error(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int RemoteLogResponse::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required .AANF_Protocol.RemoteLogResponse.ErrorCode error = 4;
    if (has_error()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->error());
    }
    
  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  _cached_size_ = total_size;
  return total_size;
}

void RemoteLogResponse::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const RemoteLogResponse* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const RemoteLogResponse*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void RemoteLogResponse::MergeFrom(const RemoteLogResponse& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from._has_bit(0)) {
      set_error(from.error());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void RemoteLogResponse::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void RemoteLogResponse::CopyFrom(const RemoteLogResponse& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool RemoteLogResponse::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;
  
  return true;
}

void RemoteLogResponse::Swap(RemoteLogResponse* other) {
  if (other != this) {
    std::swap(error_, other->error_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata RemoteLogResponse::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = RemoteLogResponse_descriptor_;
  metadata.reflection = RemoteLogResponse_reflection_;
  return metadata;
}

::google::protobuf::internal::ExtensionIdentifier< ::Protocol::PacketFormat,
    ::google::protobuf::internal::MessageTypeTraits< ::AANF_Protocol::RemoteLogRequest >, 11, false >
  remote_log_req(kRemoteLogReqFieldNumber, ::AANF_Protocol::RemoteLogRequest::default_instance());
::google::protobuf::internal::ExtensionIdentifier< ::Protocol::PacketFormat,
    ::google::protobuf::internal::MessageTypeTraits< ::AANF_Protocol::RemoteLogResponse >, 11, false >
  remote_log_rsp(kRemoteLogRspFieldNumber, ::AANF_Protocol::RemoteLogResponse::default_instance());

}  // namespace AANF_Protocol
