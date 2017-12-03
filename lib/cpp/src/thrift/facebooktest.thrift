namespace cpp facebook.fb303

enum fb_status {

  DEAD = 0,

  STARTING = 1,

  ALIVE = 2,

  STOPPING = 3,

  STOPPED = 4,

  WARNING = 5,

}

service FacebookService {

  string getName(),

//  string getVersion(),

//  fb_status getStatus(),

//  string getStatusDetails(),

//  map<string, i64> getCounters(),

//  i64 getCounter(1: string key),

 void setOption(1: string key, 2: string value),

//  string getOption(1: string key),

 // map<string, string> getOptions(),

 // string getCpuProfile(1: i32 profileDurationInSec),

  //i64 aliveSince(),

  //oneway void reinitialize(),

  oneway void shutdown(),

}
