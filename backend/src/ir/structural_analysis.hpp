/*
 * structural_analysis.hpp
 * This code is derived from the ControlTree.h and ControlTree.cpp of
 * project gpuocelot by Yongjia Zhang.
 * The original copyright of gpuocelot appears below in its entirety.
 */

/*
 * Copyright 2011
 * GEORGIA TECH RESEARCH CORPORATION
 * ALL RIGHTS RESERVED
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 * notice,   this list of conditions and the following disclaimers.
 *     * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimers in the
 *       documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of GEORGIA TECH RESEARCH CORPORATION nor the
 * names of  its contributors may be used to endorse or promote
 * products derived  from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEORGIA TECH RESEARCH CORPORATION ''AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GEORGIA TECH RESEARCH
 * CORPORATION BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You agree that the Software will not be shipped, transferred, exported,
 * or re-exported directly into any country prohibited by the United States
 * Export Administration Act and the regulations thereunder nor will be
 * used for any purpose prohibited by the Act.
 */


#ifndef __STRUCTURAL_ANALYSIS_HPP__
#define __STRUCTURAL_ANALYSIS_HPP__

#include "ir/unit.hpp"
#include "ir/function.hpp"
#include "ir/instruction.hpp"

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <set>
#define TRANSFORM_UNSTRUCTURE

namespace analysis
{
  using namespace std;
  using namespace gbe;

  enum RegionType
  {
    BasicBlock = 0,
    Block,
    IfThen,
    IfElse,
    SelfLoop,
    WhileLoop,
    NaturalLoop
  } ;

  /* control tree virtual node */
  class Node;

  typedef unordered_set<Node *> NodeSet;
  typedef list<Node *> NodeList;
  typedef std::vector<Node *> NodeVector;

  /* control tree virtual node */
  class Node
  {
  public:
    Node(RegionType rtype, const NodeList& children): has_barrier(false), mark(false), canBeHandled(true), inversePredicate(true)
    {
      this->rtype = rtype;
      this->children = children;
    }
    virtual ~Node() {}
    NodeSet& preds() { return pred; }
    NodeSet& succs() { return succ; }
    Node*& fallthrough() { return fall_through; }
    bool& hasBarrier() { return has_barrier; }
    RegionType type() { return rtype; }
    virtual ir::BasicBlock* getEntry()
    {
      return (*(children.begin()))->getEntry();
    }
    virtual ir::BasicBlock* getExit()
    {
      return (*(children.rbegin()))->getExit();
    }

  public:
    RegionType rtype;
    NodeSet pred;
    NodeSet succ;
    NodeList children;
    Node* fall_through;
    bool has_barrier;
    bool mark;
    bool canBeHandled;
    //label is for debug
    int label;
    /* inversePredicate should be false under two circumstance,
     * fallthrough is the same with succs:
     * (1) n->succs == m && node->fallthrough == m
     * node
     * | \
     * |  \
     * m<--n
     * (2) m->succs == n && node->fallthrough == n
     * node
     * | \
     * |  \
     * m-->n
     * */
    bool inversePredicate;
  };

  /* represents basic block */
  class BasicBlockNode : public Node
  {
  public:
    BasicBlockNode(ir::BasicBlock *p_bb) : Node(BasicBlock, NodeList()) { this->p_bb = p_bb; }
    virtual ~BasicBlockNode() {}
    ir::BasicBlock* getBasicBlock() { return p_bb; }
    virtual ir::BasicBlock* getEntry() { return p_bb; }
    virtual ir::BasicBlock* getExit() { return p_bb; }
    virtual ir::BasicBlock* getFirstBB() { return p_bb; }
  private:
    ir::BasicBlock *p_bb;
  };

  /* a sequence of nodes */
  class BlockNode : public Node
  {
  public:
    BlockNode(NodeList& children) : Node(Block, children) {}
    virtual ~BlockNode(){}
  };

  /* If-Then structure node */
  class IfThenNode : public Node
  {
  public:
    IfThenNode(Node* cond, Node* ifTrue) : Node(IfThen, BuildChildren(cond, ifTrue)) {}
    virtual ~IfThenNode() {}

  private:
    const NodeList BuildChildren(Node* cond, Node* ifTrue)
    {
      NodeList children;
      children.push_back(cond);
      children.push_back(ifTrue);
      return children;
    }
  };

  /* If-Else structure node */
  class IfElseNode : public Node
  {
  public:
    IfElseNode(Node* cond, Node* ifTrue, Node* ifFalse) : Node(IfElse, BuildChildren(cond, ifTrue, ifFalse)) {}
    virtual ~IfElseNode() {}

  private:
    const NodeList BuildChildren(Node* cond, Node* ifTrue, Node* ifFalse)
    {
      NodeList children;
      children.push_back(cond);
      children.push_back(ifTrue);
      children.push_back(ifFalse);
      return children;
    }
  };

  /* Self loop structure node */
  class SelfLoopNode : public Node
  {
  public:
    SelfLoopNode(Node* node) : Node(SelfLoop, BuildChildren(node)) {}
    virtual ~SelfLoopNode() {}
    virtual ir::BasicBlock* getEntry()
    {
      return (*(children.begin()))->getEntry();
    }
    virtual ir::BasicBlock* getExit()
    {
      return (*(children.begin()))->getExit();
    }

  private:
    const NodeList BuildChildren(Node *node)
    {
      NodeList children;
      children.push_back(node);
      return children;
    }
  };

  /* While loop structure node */
  class WhileLoopNode : public Node
  {
  public:
    WhileLoopNode(Node* cond, Node* execute) : Node(WhileLoop, BuildChildren(cond, execute)) {}
    virtual ~WhileLoopNode() {}
    virtual ir::BasicBlock* getEntry()
    {
      return (*(children.begin()))->getEntry();
    }
    virtual ir::BasicBlock* getExit()
    {
      return (*(children.begin()))->getExit();
    }

  private:
    const NodeList BuildChildren(Node* cond, Node* execute)
    {
      NodeList children;
      children.push_back(cond);
      children.push_back(execute);
      return children;
    }

  };

  /* Natural loop structure node */
  class NaturalLoopNode : public Node
  {
  public:
    NaturalLoopNode(const NodeList& children): Node(NaturalLoop, children){}
    virtual ~NaturalLoopNode() {}
    virtual ir::BasicBlock* getEntry()
    {
      //TODO implement it
      return NULL;
    }
    virtual ir::BasicBlock* getExit()
    {
      //TODO implement it
      return NULL;
    }
  };

  /* computes the control tree, and do the structure identification during the computation */
  class ControlTree
  {
  public:
    void analyze();

    ControlTree(ir::Function* fn) { this->fn = fn; }
    ~ControlTree();

  private:
    /* create a sequence of BasicBlockNodes, which are refered to the basic blocks in the function */
    void initializeNodes();
    /* insert a new Node, and returns the pointer of the node */
    Node* insertNode(Node *);
    /* do the structural analysis */
    void structuralAnalysis(Node * entry);
    /* do the dfs post order traverse of the current CFG */
    void DFSPostOrder(Node *start);
    /* returns true if there is a (possibly empty) path from m to k that does not pass through n */
    bool path(Node *m, Node *k, Node *n = NULL);
    /* link region node into abstract flowgraph, adjust the predecessor and successor functions, and augment the control tree */
    void reduce(Node* node,  NodeSet nodeSet);
    /* adds node to the control tree, inserts node into _post
     * at the highest-numbered position of a node in nodeSet, removes
     * the nodes in nodeSet from _post, compacts the remaining nodes at
     * the beginning of _post, and sets _postCtr to the index of node
     * in the resulting postorder */
    void compact(Node* node,  NodeSet nodeSet);
    Node* getNodesEntry() const  { return nodes_entry;}
    /* determines whether node is the entry node of an acyclic
     * control structure and returns its region. Stores in nset the set
     * of nodes in the identified control structure */
    Node* acyclicRegionType(Node* node, NodeSet& nset);
    /* determines whether node is the entry node of a cyclic
     * control structure and returns its region. Stores in nset the set
     * of nodes in the identified control structure */
    Node* cyclicRegionType(Node*, NodeList&);
    /* is this a cyclic region? */
    bool isCyclic(Node*);
    /* is this a back edge? */
    bool isBackedge(const Node*, const Node*);
    /* returns true if there is a node k such that there is a
     * (possibly empty) path from m to k that does not pass through n
     * and an edge k->n that is a back edge, and false otherwise. */
    bool pathBack(Node*, Node*);
    /* check if there is a barrier in a basic block */
    bool checkForBarrier(const ir::BasicBlock*);
    /* insert while instruction at the proper position of Node */
    void handleSelfLoopNode(Node *, ir::LabelIndex&);
    /* mark all the BasicBlockNodes of the control tree node n as status */
    void markStructuredNodes(Node *n, bool status);
    /* mark all the ir::BasicBlocks' needEndIf of n as status */
    void markNeedEndif(Node * n, bool status);
    /* mark all the ir::BasicBlocks' needIf of n as status */
    void markNeedIf(Node *, bool);
    /* insert IF instruction at the proper position of Node */
    void handleIfNode(Node *, ir::LabelIndex&, ir::LabelIndex&);
    /* insert ENDIF instruction at the proper position of Node, this Node is
       the 'then' node of identified if-then structure */
    void handleThenNode(Node *, ir::LabelIndex&);
    /* handle the then node of identified if-else structure */
    void handleThenNode2(Node *, Node *, ir::LabelIndex);
    /* insert ELSE instruction at the proper position of Node */
    void handleElseNode(Node *, ir::LabelIndex&, ir::LabelIndex&);
    /* this calls other functions to finish the handling of identified structure blocks */
    void handleStructuredNodes();
    std::set<int> getStructureBasicBlocksIndex(Node *, std::vector<ir::BasicBlock *> &);
    std::set<ir::BasicBlock *> getStructureBasicBlocks(Node*);
    /* get livein of bb */
    void getLiveIn(ir::BasicBlock& bb, std::set<ir::Register>& livein);
    /* see comment of BasicBlock::liveout in function.hpp for detail. */
    void calculateNecessaryLiveout();
    /* get the exectutive sequence of structure n */
    void getStructureSequence(Node* n, std::vector<ir::BasicBlock*> &v);
  private:
    ir::Function *fn;
    NodeVector nodes;
    Node* nodes_entry;
    unordered_map<ir::BasicBlock *, Node *> bbmap;
    NodeList post_order;
    NodeSet visited;
    NodeList::iterator post_ctr;
  };
}
#endif
