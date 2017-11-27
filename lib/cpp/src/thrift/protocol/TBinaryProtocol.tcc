/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */




#ifndef _THRIFT_PROTOCOL_TBINARYPROTOCOL_TCC_
#define _THRIFT_PROTOCOL_TBINARYPROTOCOL_TCC_ 1

#include <thrift/protocol/TBinaryProtocol.h>

#include <limits>

namespace apache {
namespace thrift {
namespace protocol {

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeMessageBegin(const std::string& name,
                                                         const TMessageType messageType,
                                                         const int32_t seqid) {
  if (this->strict_write_) { //判断是否需要强制写入版本号
    //写入协议号的目的是可以坚持客户端和服务器端是否使用相同的协议来传输的数据，保证数据格式的正确性
    int32_t version = (VERSION_1) | ((int32_t)messageType); //本版号是协议号和消息类型的或结果

    uint32_t wsize = 0; //记录写入的长度
    wsize += writeI32(version); //写版本号
    wsize += writeString(name); //写消息名称，这就是函数名称
    wsize += writeI32(seqid); //写调用序列号
    return wsize;
  } else {
    uint32_t wsize = 0;
    wsize += writeString(name);
    wsize += writeByte((int8_t)messageType);
    wsize += writeI32(seqid);
    return wsize;
  }
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeMessageEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeStructBegin(const char* name) {
  (void)name;
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeStructEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeFieldBegin(const char* name,
                                                       const TType fieldType,
                                                       const int16_t fieldId) {
  (void)name;
  uint32_t wsize = 0;
  wsize += writeByte((int8_t)fieldType);
  wsize += writeI16(fieldId);
  return wsize;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeFieldEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeFieldStop() {
  return writeByte((int8_t)T_STOP);
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeMapBegin(const TType keyType,
                                                     const TType valType,
                                                     const uint32_t size) {
  uint32_t wsize = 0;
  wsize += writeByte((int8_t)keyType);
  wsize += writeByte((int8_t)valType);
  wsize += writeI32((int32_t)size);
  return wsize;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeMapEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeListBegin(const TType elemType, const uint32_t size) {
  uint32_t wsize = 0;
  wsize += writeByte((int8_t)elemType);
  wsize += writeI32((int32_t)size);
  return wsize;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeListEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeSetBegin(const TType elemType, const uint32_t size) {
  uint32_t wsize = 0;
  wsize += writeByte((int8_t)elemType);
  wsize += writeI32((int32_t)size);
  return wsize;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeSetEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeBool(const bool value) {
  uint8_t tmp = value ? 1 : 0;
  this->trans_->write(&tmp, 1);
  return 1;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeByte(const int8_t byte) {
  this->trans_->write((uint8_t*)&byte, 1);
  return 1;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeI16(const int16_t i16) {
  int16_t net = (int16_t)ByteOrder_::toWire16(i16);
  this->trans_->write((uint8_t*)&net, 2);
  return 2;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeI32(const int32_t i32) {
  int32_t net = (int32_t)ByteOrder_::toWire32(i32);
  this->trans_->write((uint8_t*)&net, 4);
  return 4;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeI64(const int64_t i64) {
  int64_t net = (int64_t)ByteOrder_::toWire64(i64);
  this->trans_->write((uint8_t*)&net, 8);
  return 8;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeDouble(const double dub) {
  BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
  BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

  uint64_t bits = bitwise_cast<uint64_t>(dub);
  bits = ByteOrder_::toWire64(bits);
  this->trans_->write((uint8_t*)&bits, 8);
  return 8;
}

template <class Transport_, class ByteOrder_>
template <typename StrType>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeString(const StrType& str) {
  if (str.size() > static_cast<size_t>((std::numeric_limits<int32_t>::max)())) //如果字符串长度大于int32_t的最大值，就抛异常
    throw TProtocolException(TProtocolException::SIZE_LIMIT);


  uint32_t size = static_cast<uint32_t>(str.size()); //取得字符串的长度
  uint32_t result = writeI32((int32_t)size); //写入字符串的长度到服务器
  if (size > 0) {
    this->trans_->write((uint8_t*)str.data(), size); //调用具体某一个传输方式的写入函数写入字符串数据
  }
  return result + size; //返回写入的大小
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::writeBinary(const std::string& str) {
  return TBinaryProtocolT<Transport_, ByteOrder_>::writeString(str);
}

/**
 * Reading functions
 */

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readMessageBegin(std::string& name,
                                                        TMessageType& messageType,
                                                        int32_t& seqid) {
  uint32_t result = 0;
  int32_t sz;
  result += readI32(sz); //读取消息的头部（可能是协议版本号和消息类型的组合，也可能直接是消息）

  if (sz < 0) { //如果小于0（就是二进制为第一位以1开头，说明是带有协议版本号的
    // Check for correct version number
    int32_t version = sz & VERSION_MASK; //取得消息的版本号
    if (version != VERSION_1) { //如果不匹配二进制协议的版本号就抛出一个坏的协议异常
      throw TProtocolException(TProtocolException::BAD_VERSION, "Bad version identifier");
    }
    messageType = (TMessageType)(sz & 0x000000ff); //取得消息类型
    result += readString(name); //取得消息名称（也就是函数名称）
    result += readI32(seqid); //取得函数调用ID号
  } else {
    if (this->strict_read_) { //要求读协议本版号，但是这种情况是不存在协议版本号的所以抛出异常
      throw TProtocolException(TProtocolException::BAD_VERSION,
                               "No version identifier... old protocol client in strict mode?");
    } else {
      // Handle pre-versioned input
      int8_t type;
      result += readStringBody(name, sz); //读取消息名称（也就是函数名称）
      result += readByte(type);           //读取消息类型
      messageType = (TMessageType)type;
      result += readI32(seqid);           //读取函数调用ID号
    }
  }
  return result;  //返回读取数据的长度
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readMessageEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readStructBegin(std::string& name) {
  name = "";
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readStructEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readFieldBegin(std::string& name,
                                                      TType& fieldType,
                                                      int16_t& fieldId) {
  (void)name;
  uint32_t result = 0;
  int8_t type;
  result += readByte(type);
  fieldType = (TType)type;
  if (fieldType == T_STOP) {
    fieldId = 0;
    return result;
  }
  result += readI16(fieldId);
  return result;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readFieldEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readMapBegin(TType& keyType,
                                                    TType& valType,
                                                    uint32_t& size) {
  int8_t k, v;
  uint32_t result = 0;
  int32_t sizei;
  result += readByte(k);
  keyType = (TType)k;
  result += readByte(v);
  valType = (TType)v;
  result += readI32(sizei);
  if (sizei < 0) {
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  } else if (this->container_limit_ && sizei > this->container_limit_) {
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  size = (uint32_t)sizei;
  return result;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readMapEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readListBegin(TType& elemType, uint32_t& size) {
  int8_t e;
  uint32_t result = 0;
  int32_t sizei;
  result += readByte(e);
  elemType = (TType)e;
  result += readI32(sizei);
  if (sizei < 0) {
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  } else if (this->container_limit_ && sizei > this->container_limit_) {
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  size = (uint32_t)sizei;
  return result;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readListEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readSetBegin(TType& elemType, uint32_t& size) {
  int8_t e;
  uint32_t result = 0;
  int32_t sizei;
  result += readByte(e);
  elemType = (TType)e;
  result += readI32(sizei);
  if (sizei < 0) {
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  } else if (this->container_limit_ && sizei > this->container_limit_) {
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  size = (uint32_t)sizei;
  return result;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readSetEnd() {
  return 0;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readBool(bool& value) {
  uint8_t b[1];
  this->trans_->readAll(b, 1);
  value = *(int8_t*)b != 0;
  return 1;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readByte(int8_t& byte) {
  uint8_t b[1];
  this->trans_->readAll(b, 1);
  byte = *(int8_t*)b;
  return 1;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readI16(int16_t& i16) {
  union bytes {
    uint8_t b[2];
    int16_t all;
  } theBytes;
  this->trans_->readAll(theBytes.b, 2);
  i16 = (int16_t)ByteOrder_::fromWire16(theBytes.all);
  return 2;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readI32(int32_t& i32) {
  union bytes {
    uint8_t b[4];
    int32_t all;
  } theBytes;
  this->trans_->readAll(theBytes.b, 4);
  i32 = (int32_t)ByteOrder_::fromWire32(theBytes.all);
  return 4;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readI64(int64_t& i64) {
  union bytes {
    uint8_t b[8];
    int64_t all;
  } theBytes;
  this->trans_->readAll(theBytes.b, 8);
  i64 = (int64_t)ByteOrder_::fromWire64(theBytes.all);
  return 8;
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readDouble(double& dub) {
  BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
  BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

  union bytes {
    uint8_t b[8];
    uint64_t all;
  } theBytes;
  this->trans_->readAll(theBytes.b, 8);
  theBytes.all = ByteOrder_::fromWire64(theBytes.all);
  dub = bitwise_cast<double>(theBytes.all);
  return 8;
}

template <class Transport_, class ByteOrder_>
template <typename StrType>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readString(StrType& str) {
  uint32_t result;
  int32_t size;
  result = readI32(size);
  return result + readStringBody(str, size);
}

template <class Transport_, class ByteOrder_>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readBinary(std::string& str) {
  return TBinaryProtocolT<Transport_, ByteOrder_>::readString(str);
}

template <class Transport_, class ByteOrder_>
template <typename StrType>
uint32_t TBinaryProtocolT<Transport_, ByteOrder_>::readStringBody(StrType& str, int32_t size) {
  uint32_t result = 0;

  // Catch error cases
  if (size < 0) {
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  }
  if (this->string_limit_ > 0 && size > this->string_limit_) {
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }

  // Catch empty string case
  if (size == 0) {
    str.clear();
    return result;
  }

  // Try to borrow first
  const uint8_t* borrow_buf;
  uint32_t got = size;
  if ((borrow_buf = this->trans_->borrow(NULL, &got))) {
    str.assign((const char*)borrow_buf, size);
    this->trans_->consume(size);
    return size;
  }

  str.resize(size);
  this->trans_->readAll(reinterpret_cast<uint8_t*>(&str[0]), size);
  return (uint32_t)size;
}
}
}
} // apache::thrift::protocol

#endif // #ifndef _THRIFT_PROTOCOL_TBINARYPROTOCOL_TCC_
