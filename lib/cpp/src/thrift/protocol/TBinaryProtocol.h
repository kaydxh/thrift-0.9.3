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

#ifndef _THRIFT_PROTOCOL_TBINARYPROTOCOL_H_
#define _THRIFT_PROTOCOL_TBINARYPROTOCOL_H_ 1

#include <thrift/protocol/TProtocol.h>
#include <thrift/protocol/TVirtualProtocol.h>

#include <boost/shared_ptr.hpp>

namespace apache {
namespace thrift {
namespace protocol {

/**
 * The default binary protocol for thrift. Writes all data in a very basic
 * binary format, essentially just spitting out the raw bytes.
 *
 */

//这个协议是Thrift支持的默认二进制协议，它以二进制的格式写所有的数据，基本上直接发送原始数据。因为它直接从TVirtualProtocol类继承，而且是一个模板类。
//它的模板参数就是一个封装具体传输发送的类，这个类才是真正实现数据传输的。

template <class Transport_, class ByteOrder_ = TNetworkBigEndian>
class TBinaryProtocolT : public TVirtualProtocol<TBinaryProtocolT<Transport_, ByteOrder_> > {
protected:
  static const int32_t VERSION_MASK = ((int32_t)0xffff0000); //取得协议的掩码
  static const int32_t VERSION_1 = ((int32_t)0x80010000); //具体协议本版号
  // VERSION_2 (0x80020000) was taken by TDenseProtocol (which has since been removed)

public:
  TBinaryProtocolT(boost::shared_ptr<Transport_> trans)
    : TVirtualProtocol<TBinaryProtocolT<Transport_, ByteOrder_> >(trans),
      trans_(trans.get()),
      string_limit_(0),
      container_limit_(0),
      strict_read_(false),
      strict_write_(true) {}

  TBinaryProtocolT(boost::shared_ptr<Transport_> trans,
                   int32_t string_limit,
                   int32_t container_limit,
                   bool strict_read,
                   bool strict_write)
    : TVirtualProtocol<TBinaryProtocolT<Transport_, ByteOrder_> >(trans),
      trans_(trans.get()),
      string_limit_(string_limit),
      container_limit_(container_limit),
      strict_read_(strict_read),
      strict_write_(strict_write) {}

  void setStringSizeLimit(int32_t string_limit) { string_limit_ = string_limit; }

  void setContainerSizeLimit(int32_t container_limit) { container_limit_ = container_limit; }

  void setStrict(bool strict_read, bool strict_write) {
    strict_read_ = strict_read;
    strict_write_ = strict_write;
  }

  /**
   * Writing functions.
   */


/*
4(头, 正常是没有的) +  4 (version) + 4 字节(函数名字长度) + 函数名字(send) +  4字节(seqid)
    + 1 字节(类型) + 2 字节 (第几个参数) + { 1 字节 结构体字段类型 + 2 字节 序列号 + 8 字节(int64) +
                        1 字节 结构体类型 + 2 字节序列号 + 8 字节(int64)}
    + 1 字节 stop(0x00) +
 */

/*
{
    1: i64 userid
    2: i64 tid
}
*/


/*

  对这个二进制协议进行一下简单的总结。

（1）如果需要传输协议版本号，那么0-4字节就是协议版本号和消息类型；否则0-4字节就直接是消息名称（其实就是函数的名称）的长度，假设长度为len。

（2）如果0-4字节是协议版本号和消息类型，那么5-8字节就是消息名称的长度，同样假设长度为len，然后再跟着len字节的消息名称；否则就是len字节的消息名称。

（3）接下来如果没有带协议版本号的还有1字节的消息类型；

（4）然后都是4字节的请求的序列号；

（5）接着继续写入参数类型的结构体（但是二进制协议并没有真正写入，所以没有占用字节）；

（6）如果真正的有参数的话就继续一次为每一个参数写入1字节的参数类型（在前面已经给出了参数类型的定义，就是一个枚举）、2字节的参数序号和具体参数需要的长度；

*/
  

  //写4字节的版本号0x8001 0001，写4字节的函数名字长度，写函数名字，最后写4字节的序列号（默认为0x00000000）
  /*ol*/ uint32_t writeMessageBegin(const std::string& name,
                                    const TMessageType messageType,
                                    const int32_t seqid);

  //没做事情
  /*ol*/ uint32_t writeMessageEnd();


  inline uint32_t writeStructBegin(const char* name); //没做事情

  inline uint32_t writeStructEnd(); //没做事情

  //写了１个字节的字段数据类型，和2个字节字段的顺序号(thrift文件中的定义)
  inline uint32_t writeFieldBegin(const char* name, const TType fieldType, const int16_t fieldId); 

  inline uint32_t writeFieldEnd(); //没做事情

  inline uint32_t writeFieldStop(); //写一个字节的0x00 T_STOP）

  //写一个字节keyType, 一个字节valType，再写4个字节的size
  inline uint32_t writeMapBegin(const TType keyType, const TType valType, const uint32_t size);

  inline uint32_t writeMapEnd(); //没做事情

  //写一个字节的elemType， 4字节的size
  inline uint32_t writeListBegin(const TType elemType, const uint32_t size);

  inline uint32_t writeListEnd(); //没做事情

  //写一个字节的elemType， 4字节的size
  inline uint32_t writeSetBegin(const TType elemType, const uint32_t size);

  //没做事情
  inline uint32_t writeSetEnd();

  inline uint32_t writeBool(const bool value); //写一个字节，0或1

  inline uint32_t writeByte(const int8_t byte); //写一个字节

  inline uint32_t writeI16(const int16_t i16); //写二个字节


  inline uint32_t writeI32(const int32_t i32); //写四个字节

  inline uint32_t writeI64(const int64_t i64); //写八个字节

  inline uint32_t writeDouble(const double dub); //写八个字节

  //先写4字节的字符串长度，再写数据
  template <typename StrType>
  inline uint32_t writeString(const StrType& str);

  //先写４字节消息头表示字节数组长度，再写字节数组内容
  inline uint32_t writeBinary(const std::string& str);

  /**
   * Reading functions
   */

  /*ol*/ uint32_t readMessageBegin(std::string& name, TMessageType& messageType, int32_t& seqid);

  //没做事情
  /*ol*/ uint32_t readMessageEnd();

  //没做事情
  inline uint32_t readStructBegin(std::string& name);

  //没做事情
  inline uint32_t readStructEnd();

  //读取字段类型和id
  inline uint32_t readFieldBegin(std::string& name, TType& fieldType, int16_t& fieldId);

  //没做事情
  inline uint32_t readFieldEnd();

  inline uint32_t readMapBegin(TType& keyType, TType& valType, uint32_t& size);

  //没做事情
  inline uint32_t readMapEnd();

  inline uint32_t readListBegin(TType& elemType, uint32_t& size);

  //没做事情
  inline uint32_t readListEnd();

  inline uint32_t readSetBegin(TType& elemType, uint32_t& size);

  //没做事情
  inline uint32_t readSetEnd();

  inline uint32_t readBool(bool& value);
  // Provide the default readBool() implementation for std::vector<bool>
  using TVirtualProtocol<TBinaryProtocolT<Transport_, ByteOrder_> >::readBool;

  inline uint32_t readByte(int8_t& byte);

  //读取2个字节的数据，把i16赋值为读取的数据，返回2，这里是先尝试从缓存区读，没有在从传输层读取 
  inline uint32_t readI16(int16_t& i16);

  //返回4字节，并将i32设置为读取4字节的值
  inline uint32_t readI32(int32_t& i32);

  inline uint32_t readI64(int64_t& i64);

  inline uint32_t readDouble(double& dub);

   //先读4字节的长度，再读该长度的字符串
  template <typename StrType>
  inline uint32_t readString(StrType& str);

  inline uint32_t readBinary(std::string& str);

protected:
  //读取sz字节的字符串，并把读取的字符串赋值为str（首先尝试从缓存中借数据，没有在从传输层读），返回sz，
  template <typename StrType>
  uint32_t readStringBody(StrType& str, int32_t sz);

  Transport_* trans_;

  int32_t string_limit_;
  int32_t container_limit_;

  // Enforce presence of version identifier
  bool strict_read_;
  bool strict_write_;
};

typedef TBinaryProtocolT<TTransport> TBinaryProtocol;
typedef TBinaryProtocolT<TTransport, TNetworkLittleEndian> TLEBinaryProtocol;

/**
 * Constructs binary protocol handlers
 */
template <class Transport_, class ByteOrder_ = TNetworkBigEndian>
class TBinaryProtocolFactoryT : public TProtocolFactory {
public:
  TBinaryProtocolFactoryT()
    : string_limit_(0), container_limit_(0), strict_read_(false), strict_write_(true) {}

  TBinaryProtocolFactoryT(int32_t string_limit,
                          int32_t container_limit,
                          bool strict_read,
                          bool strict_write)
    : string_limit_(string_limit),
      container_limit_(container_limit),
      strict_read_(strict_read),
      strict_write_(strict_write) {}

  virtual ~TBinaryProtocolFactoryT() {}

  void setStringSizeLimit(int32_t string_limit) { string_limit_ = string_limit; }

  void setContainerSizeLimit(int32_t container_limit) { container_limit_ = container_limit; }

  void setStrict(bool strict_read, bool strict_write) {
    strict_read_ = strict_read;
    strict_write_ = strict_write;
  }

  boost::shared_ptr<TProtocol> getProtocol(boost::shared_ptr<TTransport> trans) {
    boost::shared_ptr<Transport_> specific_trans = boost::dynamic_pointer_cast<Transport_>(trans);
    TProtocol* prot;
    if (specific_trans) {
      prot = new TBinaryProtocolT<Transport_, ByteOrder_>(
        specific_trans,
        string_limit_,
        container_limit_,
        strict_read_,
        strict_write_);
    } else {
      prot = new TBinaryProtocolT<TTransport, ByteOrder_>(
        trans,
        string_limit_,
        container_limit_,
        strict_read_,
        strict_write_);
    }

    return boost::shared_ptr<TProtocol>(prot);
  }

private:
  int32_t string_limit_;
  int32_t container_limit_;
  bool strict_read_;
  bool strict_write_;
};

typedef TBinaryProtocolFactoryT<TTransport> TBinaryProtocolFactory;
typedef TBinaryProtocolFactoryT<TTransport, TNetworkLittleEndian> TLEBinaryProtocolFactory;
}
}
} // apache::thrift::protocol

#include <thrift/protocol/TBinaryProtocol.tcc>

#endif // #ifndef _THRIFT_PROTOCOL_TBINARYPROTOCOL_H_
