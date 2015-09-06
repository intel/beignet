
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_context.hpp"
#include "ir/function.hpp"
#include "ir/liveness.hpp"
#include "ir/profile.hpp"
#include "sys/cvar.hpp"
#include "sys/vector.hpp"
#include <algorithm>
#include <climits>
#include <map>

namespace gbe
{

  class SelOptimizer
  {
  public:
    SelOptimizer(uint32_t features) : features(features) {}
    virtual void run() = 0;
    virtual ~SelOptimizer() {}
  protected:
    uint32_t features;
  };

  class SelBasicBlockOptimizer : public SelOptimizer
  {
  public:
    SelBasicBlockOptimizer(uint32_t features, SelectionBlock &bb) : SelOptimizer(features), bb(bb) {}
    ~SelBasicBlockOptimizer() {}
    virtual void run();

  private:
    SelectionBlock &bb;
    static const size_t MaxTries = 1;   //the times for optimization
  };

  void SelBasicBlockOptimizer::run()
  {

  }

  class SelGlobalOptimizer : public SelOptimizer
  {
  public:
    SelGlobalOptimizer(uint32_t features) : SelOptimizer(features) {}
    ~SelGlobalOptimizer() {}
    virtual void run();
  };

  void SelGlobalOptimizer::run()
  {

  }

  void Selection::optimize()
  {
    //do basic block level optimization
    for (SelectionBlock &block : *blockList) {
      SelBasicBlockOptimizer bbopt(opt_features, block);
      bbopt.run();
    }

    //do global optimization

  }
} /* namespace gbe */
