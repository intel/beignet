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


#include "structural_analysis.hpp"

namespace analysis
{
  ControlTree::~ControlTree()
  {
    NodeVector::iterator iter = nodes.begin();
    NodeVector::iterator iter_end = nodes.end();
    while(iter != iter_end)
    {
      delete *iter;
      iter++;
    }
  }
  void ControlTree::handleSelfLoopNode(Node *loopnode, ir::LabelIndex& whileLabel)
  {
              //NodeList::iterator child_iter = (*it)->children.begin();
    ir::BasicBlock *pbb = loopnode->getExit();
    GBE_ASSERT(pbb->isLoopExit);
    ir::BasicBlock::iterator it = pbb->end();
    it--;
    if (pbb->hasExtraBra)
      it--;
    ir::BranchInstruction* pinsn = static_cast<ir::BranchInstruction *>(&*it);
    ir::Register reg = pinsn->getPredicateIndex();
    /* since this node is an while node, so we remove the BRA instruction at the bottom of the exit BB of 'node',
     * and insert WHILE instead
     */
    whileLabel = pinsn->getLabelIndex();
    ir::Instruction insn = ir::WHILE(whileLabel, reg);
    ir::Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    pbb->insertAt(it, *p_new_insn);
    pbb->whileLabel = whileLabel;
    pbb->erase(it);
  }

  /* recursive mark the bbs' variable needEndif, the bbs all belong to node.*/
  void ControlTree::markNeedIf(Node *node, bool status)
  {
    if(node->type() == BasicBlock)
    {
      ir::BasicBlock* bb = ((BasicBlockNode*)node)->getBasicBlock();
      bb->needIf = status;
      return;
    }
    NodeList::iterator it = node->children.begin();
    while(it != node->children.end())
    {
      markNeedIf(*it,status);
      it++;
    }
  }

  /* recursive mark the bbs' variable needIf, the bbs all belong to node.*/
  void ControlTree::markNeedEndif(Node *node, bool status)
  {
    if(node->type() == BasicBlock)
    {
      ir::BasicBlock* bb = ((BasicBlockNode*)node)->getBasicBlock();
      bb->needEndif = status;
      return;
    }

    NodeList::iterator it = node->children.begin();
    while(it != node->children.end())
    {
      markNeedEndif(*it, status);
      it++;
    }
  }

  /* recursive mark the bbs' variable mark, the bbs all belong to node. */
  void ControlTree::markStructuredNodes(Node *node, bool status)
  {
    if(node->type() == BasicBlock)
    {
      BasicBlockNode* pbb = static_cast<BasicBlockNode *>(node);
      pbb->getBasicBlock()->belongToStructure = true;
    }
    node->mark = status;
    NodeList::iterator it = node->children.begin();
    while(it != node->children.end())
    {
      markStructuredNodes(*it, status);
      it++;
    }
  }

  void ControlTree::handleIfNode(Node *node, ir::LabelIndex& matchingEndifLabel, ir::LabelIndex& matchingElseLabel)
  {
    ir::BasicBlock *pbb = node->getExit();
    ir::BranchInstruction* pinsn = static_cast<ir::BranchInstruction *>(pbb->getLastInstruction());
    ir::Register reg = pinsn->getPredicateIndex();
    ir::BasicBlock::iterator it = pbb->end();
    it--;
    /* since this node is an if node, so we remove the BRA instruction at the bottom of the exit BB of 'node',
     * and insert IF instead
     */
    pbb->erase(it);
    ir::Instruction insn = ir::IF(matchingElseLabel, reg, node->inversePredicate);
    ir::Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    pbb->append(*p_new_insn);
    pbb->matchingEndifLabel = matchingEndifLabel;
    pbb->matchingElseLabel = matchingElseLabel;
  }

  void ControlTree::handleThenNode(Node *node, ir::LabelIndex& endiflabel)
  {
    ir::BasicBlock *pbb = node->getExit();
    ir::BasicBlock::iterator it = pbb->end();
    it--;
    ir::Instruction *p_last_insn = pbb->getLastInstruction();

    endiflabel = fn->newLabel();
    //pbb->thisEndifLabel = endiflabel;

    ir::Instruction insn = ir::ENDIF(endiflabel);
    ir::Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    // we need to insert ENDIF before the BRA(if exists).
    bool append_bra = false;
    if((*it).getOpcode() == ir::OP_BRA)
    {
      pbb->erase(it);
      append_bra = true;
    }
    pbb->append(*p_new_insn);
    if(append_bra)
      pbb->append(*p_last_insn);
  }


  void ControlTree::handleThenNode2(Node *node, Node *elsenode, ir::LabelIndex elseBBLabel)
  {
    ir::BasicBlock *pbb = node->getExit();
    ir::BasicBlock::iterator it = pbb->end();
    it--;
    if((*it).getOpcode() == ir::OP_BRA)
      pbb->erase(it);

    if(node->getExit()->getNextBlock() == elsenode->getEntry())
      return;

    // Add an unconditional jump to 'else' block
    ir::Instruction insn = ir::BRA(elseBBLabel);
    ir::Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    pbb->append(*p_new_insn);
  }


  void ControlTree::handleElseNode(Node* node, ir::LabelIndex& elselabel, ir::LabelIndex& endiflabel)
  {
    // to insert ENDIF properly
    handleThenNode(node, endiflabel);

    ir::BasicBlock *pbb = node->getEntry();
    ir::BasicBlock::iterator it = pbb->begin();
    it++;

    elselabel = fn->newLabel();
    pbb->thisElseLabel = elselabel;

    // insert ELSE properly
    ir::Instruction insn = ir::ELSE(endiflabel);
    ir::Instruction* p_new_insn = pbb->getParent().newInstruction(insn);

    pbb->insertAt(it, *p_new_insn);
  }


  void ControlTree::handleStructuredNodes()
  {
    NodeVector::iterator it;
    NodeVector::iterator end = nodes.end();
    NodeVector::iterator begin = nodes.begin();
    it = end;
    it--;
    NodeVector::reverse_iterator rit = nodes.rbegin();
    /* structured bbs only need if and endif insn to handle the execution
     * in structure entry and exit BasicBlock, so we process the nodes backward, since
     * the node at the back of nodes is always a 'not smaller' structure then
     * the ones before it. we mark the nodes which are sub-nodes of the node
     * we are dealing with, in order to ensure we are always handling the 'biggest'
     * structures */
    while(rit != nodes.rend())
    {
      if((*rit)->type() == IfThen || (*rit)->type() == IfElse|| (*rit)->type() == SelfLoop)
      {
        if(false == (*rit)->mark && (*rit)->canBeHandled)
        {
          markStructuredNodes(*rit, true);
          /* only the entry bb of this structure needs 'if' at backend and
           * only the exit bb of this structure needs 'endif' at backend
           * see comment about needEndif and needIf at function.hpp for detail. */
          markNeedEndif(*rit, false);
          markNeedIf(*rit, false);
          ir::BasicBlock* entry = (*rit)->getEntry();
          ir::BasicBlock* eexit = (*rit)->getExit();
          entry->needIf = true;
          eexit->needEndif = true;
          entry->endifLabel = fn->newLabel();
          eexit->endifLabel = entry->endifLabel;
          eexit->isStructureExit = true;
          eexit->matchingStructureEntry = entry;
        }
      }
      rit++;
    }

    rit = nodes.rbegin();
    gbe::vector<ir::BasicBlock *> &blocks = fn->getBlocks();
    std::vector<ir::BasicBlock *> bbs;
    bbs.resize(blocks.size());

    /* here insert the bras to the BBs, which would
     * simplify the reorder of basic blocks */
    for(size_t i = 0; i < blocks.size(); ++i)
    {
      bbs[i] = blocks[i];
      if(i != blocks.size() -1 &&
         (bbs[i]->getLastInstruction()->getOpcode() != ir::OP_BRA ||
         (bbs[i]->isStructureExit && bbs[i]->isLoopExit)))
      {
        ir::Instruction insn = ir::BRA(bbs[i]->getNextBlock()->getLabelIndex());
        ir::Instruction* pNewInsn = bbs[i]->getParent().newInstruction(insn);
        bbs[i]->append(*pNewInsn);
        if (bbs[i]->isStructureExit && bbs[i]->isLoopExit)
          bbs[i]->hasExtraBra = true;
      }
    }

    /* now, reorder the basic blocks to reduce the unconditional jump we inserted whose
     * targets are the 'else' nodes. the algorithm is quite simple, just put the unstructured
     * BBs(maybe belong to another structure, but not this one) in front of the entry BB of
     * this structure in front of all the others and put the other unstructured BBs at the
     * back of the others. the sequence of structured is get through function getStructureSequence.
     */
    while(rit != nodes.rend())
    {
      if(((*rit)->type() == IfThen || (*rit)->type() == IfElse || (*rit)->type() == Block ||(*rit)->type() == SelfLoop) &&
          (*rit)->canBeHandled && (*rit)->mark == true)
      {
        markStructuredNodes(*rit, false);
        std::set<int> ns = getStructureBasicBlocksIndex(*rit, bbs);
        ir::BasicBlock *entry = (*rit)->getEntry();

        int entryIndex = *(ns.begin());
        for(size_t i=0; i<bbs.size(); ++i)
        {
          if(bbs[i] == entry)
            entryIndex = i;
        }

        std::set<int>::iterator iter = ns.begin();
        int index = *iter;

        std::vector<ir::BasicBlock *> unstruSeqHead;
        std::vector<ir::BasicBlock *> unstruSeqTail;

        iter = ns.begin();
        while(iter != ns.end())
        {
          if(index != *iter)
          {
            if(index < entryIndex)
              unstruSeqHead.push_back(bbs[index]);
            else
              unstruSeqTail.push_back(bbs[index]);
            index++;
          }
          else
          {
            index++;
            iter++;
          }
        }

        std::vector<ir::BasicBlock *> struSeq;
        getStructureSequence(*rit, struSeq);

        int firstindex = *(ns.begin());
        for(size_t i = 0; i < unstruSeqHead.size(); ++i)
          bbs[firstindex++] = unstruSeqHead[i];
        for(size_t i = 0; i < struSeq.size(); ++i)
          bbs[firstindex++] = struSeq[i];
        for(size_t i = 0; i < unstruSeqTail.size(); ++i)
          bbs[firstindex++] = unstruSeqTail[i];
      }
      rit++;
    }

   /* now, erase the BRAs inserted before whose targets are their fallthrough blocks */
    for(size_t i=0; i<bbs.size(); ++i)
    {
      if(bbs[i]->getLastInstruction()->getOpcode() == ir::OP_BRA &&
         !((ir::BranchInstruction*)(bbs[i]->getLastInstruction()))->isPredicated())
      {
        if(((ir::BranchInstruction *)bbs[i]->getLastInstruction())->getLabelIndex() == bbs[i+1]->getLabelIndex())
        {
          ir::BasicBlock::iterator it= bbs[i]->end();
          it--;

          bbs[i]->erase(it);

          if (bbs[i]->hasExtraBra)
            bbs[i]->hasExtraBra = false;
        }
      }
    }
    for(size_t i=0; i<bbs.size(); ++i)
      blocks[i] = bbs[i];

    fn->sortLabels();
    fn->computeCFG();

    it = begin;
    while(it != end)
    {
      if((*it)->canBeHandled)
      {
        switch((*it)->type())
        {
          case IfThen:
            {
              NodeList::iterator child_iter = (*it)->children.end();
              ir::LabelIndex endiflabel;
              child_iter--;
              handleThenNode(*child_iter, endiflabel); // this call would pass out the proper endiflabel for handleIfNode's use.
              child_iter--;
              handleIfNode(*child_iter, endiflabel, endiflabel);
            }
            break;

          case IfElse:
            {
              NodeList::iterator child_iter = (*it)->children.end();
              ir::LabelIndex endiflabel;
              ir::LabelIndex elselabel;
              NodeList::iterator else_node;
              child_iter--;
              else_node = child_iter;
              handleElseNode(*child_iter, elselabel, endiflabel);
              ir::LabelIndex elseBBLabel = (*child_iter)->getEntry()->getLabelIndex();
              child_iter--;
              handleThenNode2(*child_iter, *else_node, elseBBLabel);
              child_iter--;
              handleIfNode(*child_iter, endiflabel, elselabel);
            }
            break;

          case SelfLoop:
            {
              ir::LabelIndex whilelabel;
              handleSelfLoopNode(*it, whilelabel);
            }
            break;

          default:
            break;
        }
      }

      it++;
    }

  }

  void ControlTree::getStructureSequence(Node *node, std::vector<ir::BasicBlock*> &seq)
  {
    /* in the control tree, for if-then, if node is before then node; for if-else, the
     * stored sequence is if-then-else, for block structure, the stored sequence is just
     * their executed sequence. so we could just get the structure sequence by recrusive
     * calls getStructureSequence to all the elements in children one by one.
     */
    if(node->type() == BasicBlock)
    {
      seq.push_back(((BasicBlockNode *)node)->getBasicBlock());
      return;
    }

    NodeList::iterator iter = node->children.begin();
    while(iter != node->children.end())
    {
      getStructureSequence(*iter, seq);
      iter++;
    }

  }


  std::set<int> ControlTree::getStructureBasicBlocksIndex(Node* node, std::vector<ir::BasicBlock *> &bbs)
  {
    std::set<int> result;
    if(node->type() == BasicBlock)
    {
      for(size_t i=0; i<bbs.size(); i++)
      {
        if(bbs[i] == ((BasicBlockNode *)node)->getBasicBlock())
        {
          result.insert(i);
          break;
        }
      }
      return result;
    }
    NodeList::iterator iter = (node->children).begin();
    NodeList::iterator end = (node->children).end();
    while(iter != end)
    {
      std::set<int> ret = getStructureBasicBlocksIndex(*iter, bbs);
      result.insert(ret.begin(), ret.end());
      iter++;
    }
    return result;
  }


  std::set<ir::BasicBlock *> ControlTree::getStructureBasicBlocks(Node *node)
  {
    std::set<ir::BasicBlock *> result;
    if(node->type() == BasicBlock)
    {
      result.insert(((BasicBlockNode *)node)->getBasicBlock());
      return result;
    }
    NodeList::iterator iter = (node->children).begin();
    NodeList::iterator end = (node->children).end();
    while(iter != end)
    {
      std::set<ir::BasicBlock *> ret = getStructureBasicBlocks(*iter);
      result.insert(ret.begin(), ret.end());
      iter++;
    }
    return result;
  }


  Node* ControlTree::insertNode(Node *p_node)
  {
    nodes.push_back(p_node);
    return p_node;
  }


  bool ControlTree::checkForBarrier(const ir::BasicBlock* bb)
  {
    ir::BasicBlock::const_iterator iter = bb->begin();
    ir::BasicBlock::const_iterator iter_end = bb->end();
    while(iter != iter_end)
    {
      if((*iter).getOpcode() == ir::OP_SYNC)
        return true;
      iter++;
    }

    return false;
  }


  void ControlTree::getLiveIn(ir::BasicBlock& bb, std::set<ir::Register>& livein)
  {
    ir::BasicBlock::iterator iter = bb.begin();
    std::set<ir::Register> varKill;
    while(iter != bb.end())
    {
      ir::Instruction& insn = *iter;
      const uint32_t srcNum = insn.getSrcNum();
      const uint32_t dstNum = insn.getDstNum();
      for(uint32_t srcID = 0; srcID < srcNum; ++srcID)
      {
        const ir::Register reg = insn.getSrc(srcID);
        if(varKill.find(reg) == varKill.end())
          livein.insert(reg);
      }
      for(uint32_t dstID = 0; dstID < dstNum; ++dstID)
      {
        const ir::Register reg = insn.getDst(dstID);
        varKill.insert(reg);
      }

      iter++;
    }
  }

  void ControlTree::calculateNecessaryLiveout()
  {
    NodeVector::iterator iter = nodes.begin();

    while(iter != nodes.end())
    {
      switch((*iter)->type())
      {
        case IfElse:
        {
          std::set<ir::BasicBlock *> bbs;
          NodeList::iterator thenIter = (*iter)->children.begin();
          thenIter++;
          bbs = getStructureBasicBlocks(*thenIter);

          Node *elseNode = *((*iter)->children.rbegin());
          std::set<ir::Register> livein;
          getLiveIn(*(elseNode->getEntry()), livein);

          std::set<ir::BasicBlock *>::iterator bbiter = bbs.begin();
          while(bbiter != bbs.end())
          {
            (*bbiter)->liveout.insert(livein.begin(), livein.end());
            bbiter++;
          }
        }

        default:
          break;
      }
      iter++;
    }
  }


  void ControlTree::initializeNodes()
  {
    ir::BasicBlock& tmp_bb = fn->getTopBlock();
    ir::BasicBlock* p_tmp_bb = &tmp_bb;
    Node* p = NULL;

    if(NULL != p_tmp_bb)
    {
      Node *p_tmp_node = new BasicBlockNode(p_tmp_bb);
      p_tmp_node->label = p_tmp_bb->getLabelIndex();

      if(checkForBarrier(p_tmp_bb))
        p_tmp_node->hasBarrier() = true;

      nodes.push_back(p_tmp_node);
      bbmap[p_tmp_bb] = p_tmp_node;
      p_tmp_bb = p_tmp_bb->getNextBlock();
      p = p_tmp_node;
    }

    while(p_tmp_bb != NULL)
    {
      Node *p_tmp_node = new BasicBlockNode(p_tmp_bb);
      p_tmp_node->label = p_tmp_bb->getLabelIndex();

      if(checkForBarrier(p_tmp_bb))
        p_tmp_node->hasBarrier() = true;

      p->fallthrough() = p_tmp_node;
      p = p_tmp_node;
      nodes.push_back(p_tmp_node);
      bbmap[p_tmp_bb] = p_tmp_node;
      p_tmp_bb = p_tmp_bb->getNextBlock();
    }

    if(NULL != p)
      p->fallthrough() = NULL;

    p_tmp_bb = &tmp_bb;

    this->nodes_entry = bbmap[p_tmp_bb];

    while(p_tmp_bb != NULL)
    {
      ir::BlockSet::const_iterator iter_begin = p_tmp_bb->getPredecessorSet().begin();
      ir::BlockSet::const_iterator iter_end = p_tmp_bb->getPredecessorSet().end();
      while(iter_begin != iter_end)
      {
        bbmap[p_tmp_bb]->preds().insert(bbmap[*iter_begin]);
        iter_begin++;
      }

      iter_begin = p_tmp_bb->getSuccessorSet().begin();
      iter_end = p_tmp_bb->getSuccessorSet().end();
      while(iter_begin != iter_end)
      {
        bbmap[p_tmp_bb]->succs().insert(bbmap[*iter_begin]);
        iter_begin++;
      }

      p_tmp_bb = p_tmp_bb->getNextBlock();
    }
  }


  void ControlTree::DFSPostOrder(Node *start)
  {
    visited.insert(start);
    NodeSet::iterator y;
    NodeSet::iterator iter_begin = start->succs().begin();
    NodeSet::iterator iter_end = start->succs().end();
    for(y = iter_begin; y != iter_end; ++y )
    {
      if(visited.find(*y) != visited.end())
        continue;
      DFSPostOrder(*y);
    }
    post_order.push_back(start);
  }


  bool ControlTree::isCyclic(Node* node)
  {
    if(node->type() == NaturalLoop ||
       node->type() == WhileLoop ||
       node->type() == SelfLoop)
      return true;

    return false;
  }


  bool ControlTree::isBackedge(const Node* head, const Node* tail)
  {
    const Node* match[] = {head, tail};
    NodeList::iterator n = find_first_of(post_order.begin(), post_order.end(), match, match + 2);

    if(*n == head)
      return true;
    if(*n == tail)
      return false;

    return false;
  }


  bool ControlTree::pathBack(Node* m, Node* n)
  {
    for(NodeSet::const_iterator iter = n->preds().begin(); iter!= n->preds().end(); iter++)
    {
      if(isBackedge(*iter, n))
      {
        visited.clear();
        if(path(m, *iter, n))
          return true;
      }
    }

    return false;
  }

  /* this algorithm is from Muchnick's textbook(sec 7.7) (Advanced Compiler Design and Implementation) */
  Node* ControlTree::acyclicRegionType(Node* node, NodeSet& nset)
  {
    nset.clear();
    Node *n;
    bool p, s, barrier;
    NodeList nodes;

    n = node;
    p = true;
    s = (n->succs().size()==1);
    barrier = n->hasBarrier();
    while(p && s && !barrier)
    {
      if(nset.insert(n).second)
        nodes.push_back(n);
      n = *(n->succs().begin());
      barrier = n->hasBarrier();
      p = (n->preds().size() == 1);
      s = (n->succs().size() == 1);
    }

    if(p && !barrier)
    {
      if(nset.insert(n).second)
        nodes.push_back(n);
    }

    n = node;
    p = (n->preds().size() == 1);
    s = true;
    barrier = n->hasBarrier();

    while(p && s && !barrier)
    {
      if(nset.insert(n).second)
        nodes.push_front(n);
      n = *(n->preds().begin());
      barrier = n->hasBarrier();
      p = (n->preds().size() == 1);
      s = (n->succs().size() == 1);
    }

    if(s && !barrier)
    {
      if(nset.insert(n).second)
        nodes.push_front(n);
    }

    node = n;

    if(nodes.size() >=2 )
    {
      Node* p = new BlockNode(nodes);
      NodeList::iterator iter = nodes.begin();
      while(iter != nodes.end())
      {
        if((*iter)->canBeHandled == false)
        {
          p->canBeHandled = false;
          break;
        }
        iter++;
      }

      return insertNode(p);
    }

    else if(node->succs().size() == 2)
    {
      Node *m;
      m = *(node->succs().begin());
      n = *(++(node->succs().begin()));

      /* check for if node then n */
      if( n->succs().size() == 1 &&
         n->preds().size() == 1 &&
         *(n->succs().begin()) == m &&
         !n->hasBarrier() && !node->hasBarrier())
      {
        nset.clear();
        nset.insert(node);
        nset.insert(n);

        Node* p = new IfThenNode(node, n);
        if(node->fallthrough() == m)
          node->inversePredicate = false;

        if(node->canBeHandled == false || n->canBeHandled == false)
          p->canBeHandled = false;

        return insertNode(p);
      }

      /* check for if node then m */
      if(m->succs().size() == 1 &&
         m->preds().size() == 1 &&
         *(m->succs().begin()) == n &&
         !m->hasBarrier() && !node->hasBarrier())
      {
        nset.clear();
        nset.insert(node);
        nset.insert(m);

        Node* p = new IfThenNode(node, m);
        if(node->fallthrough() == n)
          node->inversePredicate = false;

        if(node->canBeHandled == false || m->canBeHandled == false)
          p->canBeHandled = false;

        return insertNode(p);
      }

      /* check for if node then n else m */
      if(m->succs().size() == 1 && n->succs().size() == 1 &&
         m->preds().size() == 1 && n->preds().size() == 1 &&
         *(m->succs().begin()) == *(n->succs().begin()) &&
         node->fallthrough() == n && !m->hasBarrier() && !n->hasBarrier() && !node->hasBarrier())
      {
        nset.clear();
        nset.insert(node);
        nset.insert(n);
        nset.insert(m);

        Node* p = new IfElseNode(node, n, m);

        if(node->canBeHandled == false ||
           m->canBeHandled == false ||
           n->canBeHandled == false)
          p->canBeHandled = false;

        return insertNode(p);
      }

      /* check for if node then m else n */
      if(m->succs().size() == 1 && n->succs().size() == 1 &&
         m->preds().size() == 1 && n->preds().size() == 1 &&
         *(m->succs().begin()) == *(n->succs().begin()) &&
         node->fallthrough() == m && !m->hasBarrier() && !n->hasBarrier() &&!node->hasBarrier())
      {
        nset.clear();
        nset.insert(node);
        nset.insert(m);
        nset.insert(n);

        Node* p = new IfElseNode(node, m, n);

        if(node->canBeHandled == false ||
           m->canBeHandled == false ||
           n->canBeHandled == false)
          p->canBeHandled = false;
        return insertNode(p);
      }
    }

    return NULL;
  }


  bool ControlTree::path(Node *from, Node *to, Node *notthrough)
  {

    if(from == notthrough || visited.find(from) != visited.end())
      return false;

    if(from == to)
      return true;

    visited.insert(from);

    for(NodeSet::const_iterator s = from->succs().begin(); s != from->succs().end(); s++)
    {
      if(path(*s, to, notthrough))
        return true;
    }

    return false;
  }


  /* this algorithm could work right, but it is quite inefficient, and
   * we are not handling any cyclic regions at this moment, so here just
   * ignore the identification of cyclic regions. */
  Node * ControlTree::cyclicRegionType(Node *node, NodeList &nset)
  {
    /* check for self-loop */
    if(nset.size() == 1)
    {
      if(node->succs().find(node) != node->succs().end())
      {
        Node* p = new SelfLoopNode(node);

        p->canBeHandled = true;
        node->getExit()->isLoopExit = true;
        return insertNode(p);
      }
      else
        return NULL;
    }

    /* check for improper region */
    for(NodeList::const_iterator m = nset.begin(); m != nset.end(); m++)
    {
      visited.clear();
      if(!path(node, *m))
        return NULL;
    }

    /* check for while loop */
    NodeList::iterator m;
    for(m = nset.begin(); m != nset.end(); ++m)
    {
      if(*m == node)
        continue;
      if(node->succs().size() == 2 && (*m)->succs().size() == 1 &&
         node->preds().size() == 2 && (*m)->preds().size() == 1)
      {
        Node* p = new WhileLoopNode(node, *m);

        p->canBeHandled = false;

        return insertNode(p);
      }
    }
    return NULL;
  }


  /* this algorithm is from Muchnick's textbook(sec 7.7) (Advanced Compiler Design and Implementation) */
  void ControlTree::reduce(Node* node,  NodeSet nodeSet)
  {
    NodeSet::iterator n;
    for(n = nodeSet.begin(); n != nodeSet.end(); n++)
    {
      NodeSet::iterator p;
      for(p = (*n)->preds().begin(); p != (*n)->preds().end(); p++)
      {
        if(nodeSet.find(*p) != nodeSet.end())
          continue;

        (*p)->succs().erase(*n);

        (*p)->succs().insert(node);
        node->preds().insert(*p);

        if((*p)->fallthrough() == *n)
          (*p)->fallthrough() = node;
      }


     NodeSet::iterator s;
     for(s = (*n)->succs().begin(); s != (*n)->succs().end(); s++)
     {
        if(nodeSet.find(*s) != nodeSet.end())
          continue;

       (*s)->preds().erase(*n);

       (*s)->preds().insert(node);
       node->succs().insert(*s);

       if((*n)->fallthrough() == *s)
         node->fallthrough() = *s;
     }
    }

    if(!isCyclic(node))
    {
      for(n = nodeSet.begin(); n != nodeSet.end(); n++)
      {
        bool shouldbreak = false;
        NodeSet::iterator p;
        for(p = (*n)->preds().begin(); p != (*n)->preds().end(); p++)
        {
          if(nodeSet.find(*p) == nodeSet.end())
            continue;

          if(isBackedge(*p, *n))
          {
            node->preds().insert(node);
            node->succs().insert(node);

            shouldbreak = true;
            break;
          }
        }

        if(shouldbreak)
          break;
      }
    }

    compact(node, nodeSet);
  }


  /* this algorithm is from Muchnick's textbook(sec 7.7) (Advanced Compiler Design and Implementation) */
  void ControlTree::compact(Node* node,  NodeSet nodeSet)
  {
    NodeList::iterator n, pos;
    for(n = post_order.begin(); n!= post_order.end() && !nodeSet.empty();)
    {
      if(!nodeSet.erase(*n))
      {
        n++;
        continue;
      }

      n = post_order.erase(n);
      pos = n;
    }

    post_ctr = post_order.insert(pos, node);
  }


  /* this algorithm is from Muchnick's textbook(sec 7.7) (Advanced Compiler Design and Implementation) */
  void ControlTree::structuralAnalysis(Node *entry)
  {
    Node* n;
    NodeSet nset;
    NodeList reachUnder;
    bool changed;
    do
    {
      changed = false;
      post_order.clear();
      visited.clear();

      DFSPostOrder(entry);
      post_ctr = post_order.begin();

      while(post_order.size() > 1 && post_ctr != post_order.end())
      {
        n = *post_ctr;
        Node* region = acyclicRegionType(n, nset);

        if( NULL != region)
        {
          changed = true;

          reduce(region, nset);

          if(nset.find(entry) != nset.end())
            entry = region;
        }
        // FIXME loop optimization is still buggy and under development, now disable it by default.
        else
        {
#if 0
          reachUnder.clear();
          nset.clear();
          for(NodeList::const_iterator m = post_order.begin(); m != post_order.end(); m++)
          {
            if(*m != n && pathBack(*m, n))
            {
              reachUnder.push_front(*m);
              nset.insert(*m);
            }
          }

          reachUnder.push_front(n);
          nset.insert(n);
          region = cyclicRegionType(n, reachUnder);

          if(NULL != region)
          {
            reduce(region, nset);
            changed = true;

            if(nset.find(entry) != nset.end())
              entry = region;
          }
          else
          {
            post_ctr++;
          }
#else
          post_ctr++;
#endif
        }
      }

      if(!changed)
        break;

    } while(post_order.size()>1);

  }

  void ControlTree::analyze()
  {
    initializeNodes();
    structuralAnalysis(nodes_entry);
    handleStructuredNodes();
    calculateNecessaryLiveout();
  }
}
