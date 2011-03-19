// Generated by the protocol buffer compiler.  DO NOT EDIT!

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "Report.pb.h"
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>

namespace AANF_Message {

namespace {

const ::google::protobuf::Descriptor* ReportRequest_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  ReportRequest_reflection_ = NULL;
const ::google::protobuf::Descriptor* ReportResponse_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  ReportResponse_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* ReportResponse_ErrorCode_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_Report_2eproto() {
  protobuf_AddDesc_Report_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "Report.proto");
  GOOGLE_CHECK(file != NULL);
  ReportRequest_descriptor_ = file->message_type(0);
  static const int ReportRequest_offsets_[6] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, report_id_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, max_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, min_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, avg_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, dev_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, cnt_),
  };
  ReportRequest_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      ReportRequest_descriptor_,
      ReportRequest::default_instance_,
      ReportRequest_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportRequest, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(ReportRequest));
  ReportResponse_descriptor_ = file->message_type(1);
  static const int ReportResponse_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportResponse, report_id_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportResponse, error_),
  };
  ReportResponse_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      ReportResponse_descriptor_,
      ReportResponse::default_instance_,
      ReportResponse_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportResponse, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ReportResponse, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(ReportResponse));
  ReportResponse_ErrorCode_descriptor_ = ReportResponse_descriptor_->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_Report_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    ReportRequest_descriptor_, &ReportRequest::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    ReportResponse_descriptor_, &ReportResponse::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_Report_2eproto() {
  delete ReportRequest::default_instance_;
  delete ReportRequest_reflection_;
  delete ReportResponse::default_instance_;
  delete ReportResponse_reflection_;
}

void protobuf_AddDesc_Report_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::AANF_Message::protobuf_AddDesc_MessageFormat_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\014Report.proto\022\014AANF_Message\032\023MessageFor"
    "mat.proto\"c\n\rReportRequest\022\021\n\treport_id\030"
    "\003 \002(\005\022\013\n\003max\030\004 \002(\003\022\013\n\003min\030\005 \002(\003\022\013\n\003avg\030\006"
    " \002(\003\022\013\n\003dev\030\007 \002(\003\022\013\n\003cnt\030\010 \002(\003\"\206\001\n\016Repor"
    "tResponse\022\021\n\treport_id\030\003 \002(\005\0225\n\005error\030\004 "
    "\002(\0162&.AANF_Message.ReportResponse.ErrorC"
    "ode\"*\n\tErrorCode\022\006\n\002OK\020\000\022\025\n\021NO_SUCH_REPO"
    "RT_ID\020\001:L\n\nreport_req\022\033.AANF_Message.Mes"
    "sageFormat\030g \001(\0132\033.AANF_Message.ReportRe"
    "quest:M\n\nreport_rsp\022\033.AANF_Message.Messa"
    "geFormat\030h \001(\0132\034.AANF_Message.ReportResp"
    "onse", 444);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "Report.proto", &protobuf_RegisterTypes);
  ReportRequest::default_instance_ = new ReportRequest();
  ReportResponse::default_instance_ = new ReportResponse();
  ::google::protobuf::internal::ExtensionSet::RegisterMessageExtension(
    &::AANF_Message::MessageFormat::default_instance(),
    103, 11, false, false,
    &::AANF_Message::ReportRequest::default_instance());
  ::google::protobuf::internal::ExtensionSet::RegisterMessageExtension(
    &::AANF_Message::MessageFormat::default_instance(),
    104, 11, false, false,
    &::AANF_Message::ReportResponse::default_instance());
  ReportRequest::default_instance_->InitAsDefaultInstance();
  ReportResponse::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_Report_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_Report_2eproto {
  StaticDescriptorInitializer_Report_2eproto() {
    protobuf_AddDesc_Report_2eproto();
  }
} static_descriptor_initializer_Report_2eproto_;


// ===================================================================

#ifndef _MSC_VER
const int ReportRequest::kReportIdFieldNumber;
const int ReportRequest::kMaxFieldNumber;
const int ReportRequest::kMinFieldNumber;
const int ReportRequest::kAvgFieldNumber;
const int ReportRequest::kDevFieldNumber;
const int ReportRequest::kCntFieldNumber;
#endif  // !_MSC_VER

ReportRequest::ReportRequest() {
  SharedCtor();
}

void ReportRequest::InitAsDefaultInstance() {
}

ReportRequest::ReportRequest(const ReportRequest& from) {
  SharedCtor();
  MergeFrom(from);
}

void ReportRequest::SharedCtor() {
  _cached_size_ = 0;
  report_id_ = 0;
  max_ = GOOGLE_LONGLONG(0);
  min_ = GOOGLE_LONGLONG(0);
  avg_ = GOOGLE_LONGLONG(0);
  dev_ = GOOGLE_LONGLONG(0);
  cnt_ = GOOGLE_LONGLONG(0);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

ReportRequest::~ReportRequest() {
  SharedDtor();
}

void ReportRequest::SharedDtor() {
  if (this != default_instance_) {
  }
}

const ::google::protobuf::Descriptor* ReportRequest::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ReportRequest_descriptor_;
}

const ReportRequest& ReportRequest::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_Report_2eproto();  return *default_instance_;
}

ReportRequest* ReportRequest::default_instance_ = NULL;

ReportRequest* ReportRequest::New() const {
  return new ReportRequest;
}

void ReportRequest::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    report_id_ = 0;
    max_ = GOOGLE_LONGLONG(0);
    min_ = GOOGLE_LONGLONG(0);
    avg_ = GOOGLE_LONGLONG(0);
    dev_ = GOOGLE_LONGLONG(0);
    cnt_ = GOOGLE_LONGLONG(0);
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool ReportRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 report_id = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt32(
              input, &report_id_));
        _set_bit(0);
        if (input->ExpectTag(32)) goto parse_max;
        break;
      }
      
      // required int64 max = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
       parse_max:
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt64(
              input, &max_));
        _set_bit(1);
        if (input->ExpectTag(40)) goto parse_min;
        break;
      }
      
      // required int64 min = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
       parse_min:
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt64(
              input, &min_));
        _set_bit(2);
        if (input->ExpectTag(48)) goto parse_avg;
        break;
      }
      
      // required int64 avg = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
       parse_avg:
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt64(
              input, &avg_));
        _set_bit(3);
        if (input->ExpectTag(56)) goto parse_dev;
        break;
      }
      
      // required int64 dev = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
       parse_dev:
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt64(
              input, &dev_));
        _set_bit(4);
        if (input->ExpectTag(64)) goto parse_cnt;
        break;
      }
      
      // required int64 cnt = 8;
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
       parse_cnt:
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt64(
              input, &cnt_));
        _set_bit(5);
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

void ReportRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  ::google::protobuf::uint8* raw_buffer = output->GetDirectBufferForNBytesAndAdvance(_cached_size_);
  if (raw_buffer != NULL) {
    ReportRequest::SerializeWithCachedSizesToArray(raw_buffer);
    return;
  }
  
  // required int32 report_id = 3;
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->report_id(), output);
  }
  
  // required int64 max = 4;
  if (_has_bit(1)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(4, this->max(), output);
  }
  
  // required int64 min = 5;
  if (_has_bit(2)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(5, this->min(), output);
  }
  
  // required int64 avg = 6;
  if (_has_bit(3)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(6, this->avg(), output);
  }
  
  // required int64 dev = 7;
  if (_has_bit(4)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(7, this->dev(), output);
  }
  
  // required int64 cnt = 8;
  if (_has_bit(5)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(8, this->cnt(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* ReportRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required int32 report_id = 3;
  if (_has_bit(0)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->report_id(), target);
  }
  
  // required int64 max = 4;
  if (_has_bit(1)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt64ToArray(4, this->max(), target);
  }
  
  // required int64 min = 5;
  if (_has_bit(2)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt64ToArray(5, this->min(), target);
  }
  
  // required int64 avg = 6;
  if (_has_bit(3)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt64ToArray(6, this->avg(), target);
  }
  
  // required int64 dev = 7;
  if (_has_bit(4)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt64ToArray(7, this->dev(), target);
  }
  
  // required int64 cnt = 8;
  if (_has_bit(5)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt64ToArray(8, this->cnt(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int ReportRequest::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 report_id = 3;
    if (has_report_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->report_id());
    }
    
    // required int64 max = 4;
    if (has_max()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->max());
    }
    
    // required int64 min = 5;
    if (has_min()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->min());
    }
    
    // required int64 avg = 6;
    if (has_avg()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->avg());
    }
    
    // required int64 dev = 7;
    if (has_dev()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->dev());
    }
    
    // required int64 cnt = 8;
    if (has_cnt()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->cnt());
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

void ReportRequest::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const ReportRequest* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const ReportRequest*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void ReportRequest::MergeFrom(const ReportRequest& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from._has_bit(0)) {
      set_report_id(from.report_id());
    }
    if (from._has_bit(1)) {
      set_max(from.max());
    }
    if (from._has_bit(2)) {
      set_min(from.min());
    }
    if (from._has_bit(3)) {
      set_avg(from.avg());
    }
    if (from._has_bit(4)) {
      set_dev(from.dev());
    }
    if (from._has_bit(5)) {
      set_cnt(from.cnt());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void ReportRequest::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ReportRequest::CopyFrom(const ReportRequest& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ReportRequest::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000003f) != 0x0000003f) return false;
  
  return true;
}

void ReportRequest::Swap(ReportRequest* other) {
  if (other != this) {
    std::swap(report_id_, other->report_id_);
    std::swap(max_, other->max_);
    std::swap(min_, other->min_);
    std::swap(avg_, other->avg_);
    std::swap(dev_, other->dev_);
    std::swap(cnt_, other->cnt_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata ReportRequest::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = ReportRequest_descriptor_;
  metadata.reflection = ReportRequest_reflection_;
  return metadata;
}


// ===================================================================

const ::google::protobuf::EnumDescriptor* ReportResponse_ErrorCode_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ReportResponse_ErrorCode_descriptor_;
}
bool ReportResponse_ErrorCode_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const ReportResponse_ErrorCode ReportResponse::OK;
const ReportResponse_ErrorCode ReportResponse::NO_SUCH_REPORT_ID;
const ReportResponse_ErrorCode ReportResponse::ErrorCode_MIN;
const ReportResponse_ErrorCode ReportResponse::ErrorCode_MAX;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int ReportResponse::kReportIdFieldNumber;
const int ReportResponse::kErrorFieldNumber;
#endif  // !_MSC_VER

ReportResponse::ReportResponse() {
  SharedCtor();
}

void ReportResponse::InitAsDefaultInstance() {
}

ReportResponse::ReportResponse(const ReportResponse& from) {
  SharedCtor();
  MergeFrom(from);
}

void ReportResponse::SharedCtor() {
  _cached_size_ = 0;
  report_id_ = 0;
  error_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

ReportResponse::~ReportResponse() {
  SharedDtor();
}

void ReportResponse::SharedDtor() {
  if (this != default_instance_) {
  }
}

const ::google::protobuf::Descriptor* ReportResponse::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ReportResponse_descriptor_;
}

const ReportResponse& ReportResponse::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_Report_2eproto();  return *default_instance_;
}

ReportResponse* ReportResponse::default_instance_ = NULL;

ReportResponse* ReportResponse::New() const {
  return new ReportResponse;
}

void ReportResponse::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    report_id_ = 0;
    error_ = 0;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool ReportResponse::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 report_id = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
        DO_(::google::protobuf::internal::WireFormatLite::ReadInt32(
              input, &report_id_));
        _set_bit(0);
        if (input->ExpectTag(32)) goto parse_error;
        break;
      }
      
      // required .AANF_Message.ReportResponse.ErrorCode error = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) !=
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          goto handle_uninterpreted;
        }
       parse_error:
        int value;
        DO_(::google::protobuf::internal::WireFormatLite::ReadEnum(input, &value));
        if (::AANF_Message::ReportResponse_ErrorCode_IsValid(value)) {
          set_error(static_cast< ::AANF_Message::ReportResponse_ErrorCode >(value));
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

void ReportResponse::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  ::google::protobuf::uint8* raw_buffer = output->GetDirectBufferForNBytesAndAdvance(_cached_size_);
  if (raw_buffer != NULL) {
    ReportResponse::SerializeWithCachedSizesToArray(raw_buffer);
    return;
  }
  
  // required int32 report_id = 3;
  if (_has_bit(0)) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->report_id(), output);
  }
  
  // required .AANF_Message.ReportResponse.ErrorCode error = 4;
  if (_has_bit(1)) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      4, this->error(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* ReportResponse::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required int32 report_id = 3;
  if (_has_bit(0)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->report_id(), target);
  }
  
  // required .AANF_Message.ReportResponse.ErrorCode error = 4;
  if (_has_bit(1)) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      4, this->error(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int ReportResponse::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 report_id = 3;
    if (has_report_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->report_id());
    }
    
    // required .AANF_Message.ReportResponse.ErrorCode error = 4;
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

void ReportResponse::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const ReportResponse* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const ReportResponse*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void ReportResponse::MergeFrom(const ReportResponse& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from._has_bit(0)) {
      set_report_id(from.report_id());
    }
    if (from._has_bit(1)) {
      set_error(from.error());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void ReportResponse::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ReportResponse::CopyFrom(const ReportResponse& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ReportResponse::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;
  
  return true;
}

void ReportResponse::Swap(ReportResponse* other) {
  if (other != this) {
    std::swap(report_id_, other->report_id_);
    std::swap(error_, other->error_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata ReportResponse::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = ReportResponse_descriptor_;
  metadata.reflection = ReportResponse_reflection_;
  return metadata;
}

::google::protobuf::internal::ExtensionIdentifier< ::AANF_Message::MessageFormat,
    ::google::protobuf::internal::MessageTypeTraits< ::AANF_Message::ReportRequest >, 11, false >
  report_req(kReportReqFieldNumber, ::AANF_Message::ReportRequest::default_instance());
::google::protobuf::internal::ExtensionIdentifier< ::AANF_Message::MessageFormat,
    ::google::protobuf::internal::MessageTypeTraits< ::AANF_Message::ReportResponse >, 11, false >
  report_rsp(kReportRspFieldNumber, ::AANF_Message::ReportResponse::default_instance());

}  // namespace AANF_Message
