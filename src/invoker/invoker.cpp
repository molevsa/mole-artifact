#include "istool/invoker/invoker.h"

#include "glog/logging.h"

InvokeConfig::~InvokeConfig() {
  for (auto &item : item_map) delete item.second;
}

InvokeConfig::InvokeConfigItem::~InvokeConfigItem() { free_operator(data); }