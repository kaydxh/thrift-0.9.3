/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef FacebookService_H
#define FacebookService_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "facebooktest_types.h"

namespace facebook { namespace fb303 {

#ifdef _WIN32
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

//除了这个类多了一个虚析构函数，其他函数就是IDL中定义的
class FacebookServiceIf {
 public:
  virtual ~FacebookServiceIf() {}
  virtual void getName(std::string& _return) = 0;
  virtual void setOption(const std::string& key, const std::string& value) = 0;
  virtual void shutdown() = 0;
};

class FacebookServiceIfFactory {
 public:
  typedef FacebookServiceIf Handler;

  virtual ~FacebookServiceIfFactory() {}

  virtual FacebookServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(FacebookServiceIf* /* handler */) = 0;
};

class FacebookServiceIfSingletonFactory : virtual public FacebookServiceIfFactory {
 public:
  FacebookServiceIfSingletonFactory(const boost::shared_ptr<FacebookServiceIf>& iface) : iface_(iface) {}
  virtual ~FacebookServiceIfSingletonFactory() {}

  virtual FacebookServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(FacebookServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<FacebookServiceIf> iface_;
};

//这个是抽象类FacebookServiceIf的空实现（就是所有方法都没有做具体的事情），这样做的好处就是我们需要重写一些函数的时候只需要关注我们需要写的函数，而不是重写所有函数

class FacebookServiceNull : virtual public FacebookServiceIf {
 public:
  virtual ~FacebookServiceNull() {}
  void getName(std::string& /* _return */) {
    return;
  }
  void setOption(const std::string& /* key */, const std::string& /* value */) {
    return;
  }
  void shutdown() {
    return;
  }
};

//封装每一个函数参数的相应类，就是一个函数的参数都用一个类来封装定义，函数的返回值也是这样处理
//这样做的目的是统一远程调用的实现接口，因为传递参数都只需要这个封装类的对象就可以了

class FacebookService_getName_args {
 public:

  FacebookService_getName_args(const FacebookService_getName_args&);
  FacebookService_getName_args& operator=(const FacebookService_getName_args&);
  FacebookService_getName_args() {
  }

  virtual ~FacebookService_getName_args() throw();

  bool operator == (const FacebookService_getName_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const FacebookService_getName_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FacebookService_getName_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FacebookService_getName_pargs {
 public:


  virtual ~FacebookService_getName_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FacebookService_getName_result__isset {
  _FacebookService_getName_result__isset() : success(false) {}
  bool success :1;
} _FacebookService_getName_result__isset;

class FacebookService_getName_result {
 public:

  FacebookService_getName_result(const FacebookService_getName_result&);
  FacebookService_getName_result& operator=(const FacebookService_getName_result&);
  FacebookService_getName_result() : success() {
  }

  virtual ~FacebookService_getName_result() throw();
  std::string success;

  _FacebookService_getName_result__isset __isset;

  void __set_success(const std::string& val);

  bool operator == (const FacebookService_getName_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const FacebookService_getName_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FacebookService_getName_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FacebookService_getName_presult__isset {
  _FacebookService_getName_presult__isset() : success(false) {}
  bool success :1;
} _FacebookService_getName_presult__isset;

class FacebookService_getName_presult {
 public:


  virtual ~FacebookService_getName_presult() throw();
  std::string* success;

  _FacebookService_getName_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _FacebookService_setOption_args__isset {
  _FacebookService_setOption_args__isset() : key(false), value(false) {}
  bool key :1;
  bool value :1;
} _FacebookService_setOption_args__isset;

class FacebookService_setOption_args {
 public:

  FacebookService_setOption_args(const FacebookService_setOption_args&);
  FacebookService_setOption_args& operator=(const FacebookService_setOption_args&);
  FacebookService_setOption_args() : key(), value() {
  }

  virtual ~FacebookService_setOption_args() throw();
  std::string key;
  std::string value;

  _FacebookService_setOption_args__isset __isset;

  void __set_key(const std::string& val);

  void __set_value(const std::string& val);

  bool operator == (const FacebookService_setOption_args & rhs) const
  {
    if (!(key == rhs.key))
      return false;
    if (!(value == rhs.value))
      return false;
    return true;
  }
  bool operator != (const FacebookService_setOption_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FacebookService_setOption_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FacebookService_setOption_pargs {
 public:


  virtual ~FacebookService_setOption_pargs() throw();
  const std::string* key;
  const std::string* value;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FacebookService_setOption_result {
 public:

  FacebookService_setOption_result(const FacebookService_setOption_result&);
  FacebookService_setOption_result& operator=(const FacebookService_setOption_result&);
  FacebookService_setOption_result() {
  }

  virtual ~FacebookService_setOption_result() throw();

  bool operator == (const FacebookService_setOption_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const FacebookService_setOption_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FacebookService_setOption_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FacebookService_setOption_presult {
 public:


  virtual ~FacebookService_setOption_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class FacebookService_shutdown_args {
 public:

  FacebookService_shutdown_args(const FacebookService_shutdown_args&);
  FacebookService_shutdown_args& operator=(const FacebookService_shutdown_args&);
  FacebookService_shutdown_args() {
  }

  virtual ~FacebookService_shutdown_args() throw();

  bool operator == (const FacebookService_shutdown_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const FacebookService_shutdown_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FacebookService_shutdown_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FacebookService_shutdown_pargs {
 public:


  virtual ~FacebookService_shutdown_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

//客户端的类，继承服务抽象类

class FacebookServiceClient : virtual public FacebookServiceIf {
 public:
  FacebookServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  FacebookServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getName(std::string& _return);
  void send_getName();
  void recv_getName(std::string& _return);
  void setOption(const std::string& key, const std::string& value);
  void send_setOption(const std::string& key, const std::string& value);
  void recv_setOption();
  void shutdown();
  void send_shutdown();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class FacebookServiceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<FacebookServiceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (FacebookServiceProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_getName(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_setOption(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_shutdown(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  FacebookServiceProcessor(boost::shared_ptr<FacebookServiceIf> iface) :
    iface_(iface) {
    processMap_["getName"] = &FacebookServiceProcessor::process_getName;
    processMap_["setOption"] = &FacebookServiceProcessor::process_setOption;
    processMap_["shutdown"] = &FacebookServiceProcessor::process_shutdown;
  }

  virtual ~FacebookServiceProcessor() {}
};

class FacebookServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  FacebookServiceProcessorFactory(const ::boost::shared_ptr< FacebookServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< FacebookServiceIfFactory > handlerFactory_;
};

class FacebookServiceMultiface : virtual public FacebookServiceIf {
 public:
  FacebookServiceMultiface(std::vector<boost::shared_ptr<FacebookServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~FacebookServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<FacebookServiceIf> > ifaces_;
  FacebookServiceMultiface() {}
  void add(boost::shared_ptr<FacebookServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void getName(std::string& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getName(_return);
    }
    ifaces_[i]->getName(_return);
    return;
  }

  void setOption(const std::string& key, const std::string& value) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->setOption(key, value);
    }
    ifaces_[i]->setOption(key, value);
  }

  void shutdown() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->shutdown();
    }
    ifaces_[i]->shutdown();
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class FacebookServiceConcurrentClient : virtual public FacebookServiceIf {
 public:
  FacebookServiceConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  FacebookServiceConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getName(std::string& _return);
  int32_t send_getName();
  void recv_getName(std::string& _return, const int32_t seqid);
  void setOption(const std::string& key, const std::string& value);
  int32_t send_setOption(const std::string& key, const std::string& value);
  void recv_setOption(const int32_t seqid);
  void shutdown();
  void send_shutdown();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _WIN32
  #pragma warning( pop )
#endif

}} // namespace

#endif
