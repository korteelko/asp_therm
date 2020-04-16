#include "inode_imp.h"

json_test_node<> *json_test_factory::GetNodeInitializer() {
  json_test_node<> *node = new json_test_node<>();
  if (node)
    node->data.factory_num = factory_num;
  return node;
}
