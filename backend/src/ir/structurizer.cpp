/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "structurizer.hpp"
#include "sys/cvar.hpp"

using namespace llvm;
namespace gbe {
namespace ir {
  CFGStructurizer::~CFGStructurizer()
  {
    BlockVector::iterator iter = blocks.begin();
    BlockVector::iterator iter_end = blocks.end();
    while(iter != iter_end)
    {
      delete *iter;
      iter++;
    }
  }

  void CFGStructurizer::handleSelfLoopBlock(Block *loopblock, LabelIndex& whileLabel)
  {
    //BlockList::iterator child_iter = (*it)->children.begin();
    BasicBlock *pbb = loopblock->getExit();
    GBE_ASSERT(pbb->isLoopExit);
    BasicBlock::iterator it = pbb->end();
    it--;
    if (pbb->hasExtraBra)
      it--;
    BranchInstruction* pinsn = static_cast<BranchInstruction *>(&*it);

    if(!pinsn->isPredicated()){
      std::cout << "WARNING:" << "endless loop detected!" << std::endl;
      return;
    }
    Register reg = pinsn->getPredicateIndex();
    /* since this block is an while block, so we remove the BRA instruction at the bottom of the exit BB of 'block',
     * and insert WHILE instead
     */
    whileLabel = pinsn->getLabelIndex();
    Instruction insn = WHILE(whileLabel, reg);
    Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    pbb->insertAt(it, *p_new_insn);
    pbb->whileLabel = whileLabel;
    it->remove();
  }

  /* recursive mark the bbs' variable needEndif*/
  void CFGStructurizer::markNeedIf(Block *block, bool status)
  {
    if(block->type() == SingleBlockType)
    {
      BasicBlock* bb = ((SimpleBlock*)block)->getBasicBlock();
      bb->needIf = status;
      return;
    }
    BlockList::iterator it = block->children.begin();
    while(it != block->children.end())
    {
      markNeedIf(*it,status);
      it++;
    }
  }

  /* recursive mark the bbs' variable needIf*/
  void CFGStructurizer::markNeedEndif(Block *block, bool status)
  {
    if(block->type() == SingleBlockType)
    {
      BasicBlock* bb = ((SimpleBlock*)block)->getBasicBlock();
      bb->needEndif = status;
      return;
    }

    BlockList::iterator it = block->children.begin();
    while(it != block->children.end())
    {
      markNeedEndif(*it, status);
      it++;
    }
  }

  /* recursive mark the bbs' variable mark*/
  void CFGStructurizer::markStructuredBlocks(Block *block, bool status)
  {
    if(block->type() == SingleBlockType)
    {
      SimpleBlock* pbb = static_cast<SimpleBlock*>(block);
      pbb->getBasicBlock()->belongToStructure = true;
    }
    block->mark = status;
    BlockList::iterator it = block->children.begin();
    while(it != block->children.end())
    {
      markStructuredBlocks(*it, status);
      it++;
    }
  }

  void CFGStructurizer::handleIfBlock(Block *block, LabelIndex& matchingEndifLabel, LabelIndex& matchingElseLabel)
  {
    BasicBlock *pbb = block->getExit();
    BranchInstruction* pinsn = static_cast<BranchInstruction *>(pbb->getLastInstruction());
    Register reg = pinsn->getPredicateIndex();
    BasicBlock::iterator it = pbb->end();
    it--;
    /* since this block is an if block, so we remove the BRA instruction at the bottom of the exit BB of 'block',
     * and insert IF instead
     */
    it->remove();
    Instruction insn = IF(matchingElseLabel, reg, block->inversePredicate);
    Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    pbb->append(*p_new_insn);
    pbb->matchingEndifLabel = matchingEndifLabel;
    pbb->matchingElseLabel = matchingElseLabel;
  }

  void CFGStructurizer::handleThenBlock(Block * block, LabelIndex& endiflabel)
  {
    BasicBlock *pbb = block->getExit();
    BasicBlock::iterator it = pbb->end();
    it--;
    Instruction *p_last_insn = pbb->getLastInstruction();

    endiflabel = fn->newLabel();
    //pbb->thisEndifLabel = endiflabel;

    Instruction insn = ENDIF(endiflabel);
    Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    // we need to insert ENDIF before the BRA(if exists).
    bool append_bra = false;
    if((*it).getOpcode() == OP_BRA)
    {
      pbb->erase(it);
      append_bra = true;
    }
    pbb->append(*p_new_insn);
    if(append_bra)
      pbb->append(*p_last_insn);
  }

  void CFGStructurizer::handleThenBlock2(Block *block, Block *elseblock, LabelIndex elseBBLabel)
  {
    BasicBlock *pbb = block->getExit();
    BasicBlock::iterator it = pbb->end();
    it--;
    if((*it).getOpcode() == OP_BRA)
      it->remove();

    if(block->getExit()->getNextBlock() == elseblock->getEntry())
      return;

    // Add an unconditional jump to 'else' block
    Instruction insn = BRA(elseBBLabel);
    Instruction* p_new_insn = pbb->getParent().newInstruction(insn);
    pbb->append(*p_new_insn);
  }

  void CFGStructurizer::handleElseBlock(Block * block, LabelIndex& elselabel, LabelIndex& endiflabel)
  {
    // to insert ENDIF properly
    handleThenBlock(block, endiflabel);

    BasicBlock *pbb = block->getEntry();
    BasicBlock::iterator it = pbb->begin();
    it++;

    elselabel = fn->newLabel();
    pbb->thisElseLabel = elselabel;

    // insert ELSE properly
    Instruction insn = ELSE(endiflabel);
    Instruction* p_new_insn = pbb->getParent().newInstruction(insn);

    pbb->insertAt(it, *p_new_insn);
  }

  void CFGStructurizer::handleStructuredBlocks()
  {
    BlockVector::iterator it;
    BlockVector::iterator end = blocks.end();
    BlockVector::iterator begin = blocks.begin();
    it = end;
    it--;
    BlockVector::reverse_iterator rit = blocks.rbegin();
    /* structured bbs only need if and endif insn to handle the execution
     * in structure entry and exit BasicBlock, so we process the blocks backward, since
     * the block at the back of blocks is always a 'not smaller' structure then
     * the ones before it. we mark the blocks which are sub-blocks of the block
     * we are dealing with, in order to ensure we are always handling the 'biggest'
     * structures */
    while(rit != blocks.rend())
    {
      if((*rit)->type() == IfThenType || (*rit)->type() == IfElseType|| (*rit)->type() == SelfLoopType)
      {
        if(false == (*rit)->mark && (*rit)->canBeHandled)
        {
          markStructuredBlocks(*rit, true);
          /* only the entry bb of this structure needs 'if' at backend and
           * only the exit bb of this structure needs 'endif' at backend
           * see comment about needEndif and needIf at function.hpp for detail. */
          markNeedEndif(*rit, false);
          markNeedIf(*rit, false);
          BasicBlock* entry = (*rit)->getEntry();
          BasicBlock* eexit = (*rit)->getExit();
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

    rit = blocks.rbegin();
    gbe::vector<BasicBlock *> &bblocks = fn->getBlocks();
    std::vector<BasicBlock *> bbs;
    bbs.resize(bblocks.size());

    /* here insert the bras to the BBs, which would
     * simplify the reorder of basic blocks */
    for(size_t i = 0; i < bblocks.size(); ++i)
    {
      bbs[i] = bblocks[i];
      if(i != bblocks.size() -1 &&
         (bbs[i]->getLastInstruction()->getOpcode() != OP_BRA ||
         (bbs[i]->isStructureExit && bbs[i]->isLoopExit)))
      {
        Instruction insn = BRA(bbs[i]->getNextBlock()->getLabelIndex());
        Instruction* pNewInsn = bbs[i]->getParent().newInstruction(insn);
        bbs[i]->append(*pNewInsn);
        if (bbs[i]->isStructureExit && bbs[i]->isLoopExit)
          bbs[i]->hasExtraBra = true;
      }
    }

    /* now, reorder the basic blocks to reduce the unconditional jump we inserted whose
     * targets are the 'else' blocks. the algorithm is quite simple, just put the unstructured
     * BBs(maybe belong to another structure, but not this one) in front of the entry BB of
     * this structure in front of all the others and put the other unstructured BBs at the
     * back of the others. the sequence of structured is get through function getStructureSequence.
     */
    while(rit != blocks.rend())
    {
      if(((*rit)->type() == IfThenType || (*rit)->type() == IfElseType || (*rit)->type() == SerialBlockType ||(*rit)->type() == SelfLoopType) &&
          (*rit)->canBeHandled && (*rit)->mark == true)
      {
        markStructuredBlocks(*rit, false);
        std::set<int> ns = getStructureBasicBlocksIndex(*rit, bbs);
        BasicBlock *entry = (*rit)->getEntry();

        int entryIndex = *(ns.begin());
        for(size_t i=0; i<bbs.size(); ++i)
        {
          if(bbs[i] == entry)
            entryIndex = i;
        }

        std::set<int>::iterator iter = ns.begin();
        int index = *iter;

        std::vector<BasicBlock *> unstruSeqHead;
        std::vector<BasicBlock *> unstruSeqTail;

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

        std::vector<BasicBlock *> struSeq;
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
      if(bbs[i]->getLastInstruction()->getOpcode() == OP_BRA &&
         !((BranchInstruction*)(bbs[i]->getLastInstruction()))->isPredicated())
      {
        if(((BranchInstruction *)bbs[i]->getLastInstruction())->getLabelIndex() == bbs[i+1]->getLabelIndex())
        {
          BasicBlock::iterator it= bbs[i]->end();
          it--;
          it->remove();

          if (bbs[i]->hasExtraBra)
            bbs[i]->hasExtraBra = false;
        }
      }
    }
    for(size_t i=0; i<bbs.size(); ++i)
      bblocks[i] = bbs[i];

    fn->sortLabels();
    fn->computeCFG();

    it = begin;
    while(it != end)
    {
      if((*it)->canBeHandled)
      {
        switch((*it)->type())
        {
          case IfThenType:
            {
              BlockList::iterator child_iter = (*it)->children.end();
              LabelIndex endiflabel;
              child_iter--;
              handleThenBlock(*child_iter, endiflabel); // this call would pass out the proper endiflabel for handleIfBlock's use.
              child_iter--;
              handleIfBlock(*child_iter, endiflabel, endiflabel);
            }
            break;

          case IfElseType:
            {
              BlockList::iterator child_iter = (*it)->children.end();
              LabelIndex endiflabel;
              LabelIndex elselabel;
              BlockList::iterator else_block;
              child_iter--;
              else_block= child_iter;
              handleElseBlock(*child_iter, elselabel, endiflabel);
              LabelIndex elseBBLabel = (*child_iter)->getEntry()->getLabelIndex();
              child_iter--;
              handleThenBlock2(*child_iter, *else_block, elseBBLabel);
              child_iter--;
              handleIfBlock(*child_iter, endiflabel, elselabel);
            }
            break;

          case SelfLoopType:
            {
              LabelIndex whilelabel;
              handleSelfLoopBlock(*it, whilelabel);
            }
            break;

          default:
            break;
        }
      }

      it++;
    }
  }

  void CFGStructurizer::getStructureSequence(Block *block, std::vector<BasicBlock*> &seq)
  {
    /* in the control tree, for if-then, if block is before then block; for if-else, the
     * stored sequence is if-then-else, for block structure, the stored sequence is just
     * their executed sequence. so we could just get the structure sequence by recrusive
     * calls getStructureSequence to all the elements in children one by one.
     */
    if(block->type() == SingleBlockType)
    {
      seq.push_back(((SimpleBlock*)block)->getBasicBlock());
      return;
    }

    BlockList::iterator iter = block->children.begin();
    while(iter != block->children.end())
    {
      getStructureSequence(*iter, seq);
      iter++;
    }
  }

  std::set<int> CFGStructurizer::getStructureBasicBlocksIndex(Block* block, std::vector<BasicBlock *> &bbs)
  {
    std::set<int> result;
    if(block->type() == SingleBlockType)
    {
      for(size_t i=0; i<bbs.size(); i++)
      {
        if(bbs[i] == ((SimpleBlock*)block)->getBasicBlock())
        {
          result.insert(i);
          break;
        }
      }
      return result;
    }
    BlockList::iterator iter = (block->children).begin();
    BlockList::iterator end = (block->children).end();
    while(iter != end)
    {
      std::set<int> ret = getStructureBasicBlocksIndex(*iter, bbs);
      result.insert(ret.begin(), ret.end());
      iter++;
    }
    return result;
  }

  std::set<BasicBlock *> CFGStructurizer::getStructureBasicBlocks(Block *block)
  {
    std::set<BasicBlock *> result;
    if(block->type() == SingleBlockType)
    {
      result.insert(((SimpleBlock*)block)->getBasicBlock());
      return result;
    }
    BlockList::iterator iter = (block->children).begin();
    BlockList::iterator end = (block->children).end();
    while(iter != end)
    {
      std::set<BasicBlock *> ret = getStructureBasicBlocks(*iter);
      result.insert(ret.begin(), ret.end());
      iter++;
    }
    return result;
  }

  Block* CFGStructurizer::insertBlock(Block *p_block)
  {
    blocks.push_back(p_block);
    return p_block;
  }

  void CFGStructurizer::collectInsnNum(Block* block, const BasicBlock* bb)
  {
    BasicBlock::const_iterator iter = bb->begin();
    BasicBlock::const_iterator iter_end = bb->end();
    while(iter != iter_end)
    {
      block->insnNum++;
      iter++;
    }
  }

  bool CFGStructurizer::checkForBarrier(const BasicBlock* bb)
  {
    BasicBlock::const_iterator iter = bb->begin();
    BasicBlock::const_iterator iter_end = bb->end();
    while(iter != iter_end)
    {
      if((*iter).getOpcode() == OP_SYNC)
        return true;
      iter++;
    }

    return false;
  }

  void CFGStructurizer::getLiveIn(BasicBlock& bb, std::set<Register>& livein)
  {
    BasicBlock::iterator iter = bb.begin();
    std::set<Register> varKill;
    while(iter != bb.end())
    {
      Instruction& insn = *iter;
      const uint32_t srcNum = insn.getSrcNum();
      const uint32_t dstNum = insn.getDstNum();
      for(uint32_t srcID = 0; srcID < srcNum; ++srcID)
      {
        const Register reg = insn.getSrc(srcID);
        if(varKill.find(reg) == varKill.end())
          livein.insert(reg);
      }
      for(uint32_t dstID = 0; dstID < dstNum; ++dstID)
      {
        const Register reg = insn.getDst(dstID);
        varKill.insert(reg);
      }

      iter++;
    }
  }

  void CFGStructurizer::calculateNecessaryLiveout()
  {
    BlockVector::iterator iter = blocks.begin();

    while(iter != blocks.end())
    {
      switch((*iter)->type())
      {
        case IfElseType:
        {
          std::set<BasicBlock *> bbs;
          BlockList::iterator thenIter = (*iter)->children.begin();
          thenIter++;
          bbs = getStructureBasicBlocks(*thenIter);

          Block *elseblock = *((*iter)->children.rbegin());
          std::set<Register> livein;
          getLiveIn(*(elseblock->getEntry()), livein);

          std::set<BasicBlock *>::iterator bbiter = bbs.begin();
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

  void CFGStructurizer::initializeBlocks()
  {
    BasicBlock& tmp_bb = fn->getTopBlock();
    BasicBlock* p_tmp_bb = &tmp_bb;
    Block* p = NULL;

    if(NULL != p_tmp_bb)
    {
      Block *p_tmp_block = new SimpleBlock(p_tmp_bb);
      p_tmp_block->label = p_tmp_bb->getLabelIndex();

      if(checkForBarrier(p_tmp_bb))
        p_tmp_block->hasBarrier() = true;

      blocks.push_back(p_tmp_block);
      bbmap[p_tmp_bb] = p_tmp_block;
      bTobbmap[p_tmp_block] = p_tmp_bb;
      p_tmp_bb = p_tmp_bb->getNextBlock();
      p = p_tmp_block;
    }

    while(p_tmp_bb != NULL)
    {
      Block *p_tmp_block = new SimpleBlock(p_tmp_bb);
      p_tmp_block->label = p_tmp_bb->getLabelIndex();

      if(checkForBarrier(p_tmp_bb))
        p_tmp_block->hasBarrier() = true;

      p->fallthrough() = p_tmp_block;
      p = p_tmp_block;
      blocks.push_back(p_tmp_block);
      bbmap[p_tmp_bb] = p_tmp_block;
      bTobbmap[p_tmp_block] = p_tmp_bb;
      p_tmp_bb = p_tmp_bb->getNextBlock();
    }

    if(NULL != p)
      p->fallthrough() = NULL;

    p_tmp_bb = &tmp_bb;

    this->blocks_entry = bbmap[p_tmp_bb];

    while(p_tmp_bb != NULL)
    {
      BlockSet::const_iterator iter_begin = p_tmp_bb->getPredecessorSet().begin();
      BlockSet::const_iterator iter_end = p_tmp_bb->getPredecessorSet().end();
      while(iter_begin != iter_end)
      {
        bbmap[p_tmp_bb]->predecessors().insert(bbmap[*iter_begin]);
        iter_begin++;
      }

      iter_begin = p_tmp_bb->getSuccessorSet().begin();
      iter_end = p_tmp_bb->getSuccessorSet().end();
      while(iter_begin != iter_end)
      {
        bbmap[p_tmp_bb]->successors().insert(bbmap[*iter_begin]);
        iter_begin++;
      }

      p_tmp_bb = p_tmp_bb->getNextBlock();
    }

    //copy the sequenced blocks to orderedBlks.
    loops = fn->getLoops();
    fn->foreachBlock([&](ir::BasicBlock &bb){
        orderedBlks.push_back(bbmap[&bb]);
        collectInsnNum(bbmap[&bb], &bb);
        });
  }

  void CFGStructurizer::outBlockTypes(BlockType type)
  {
    if(type == SerialBlockType)
        std::cout << " T:["<< "Serial" <<"]"<< std::endl;
    else if(type == IfThenType)
        std::cout << " T:["<< "IfThen" <<"]"<< std::endl;
    else if(type == IfElseType)
        std::cout << " T:["<< "IfElse" <<"]"<< std::endl;
    else if(type == SelfLoopType)
        std::cout << " T:["<< "SelfLoop" <<"]"<< std::endl;
    else
        std::cout << " T:["<< "BasicBlock" <<"]"<< std::endl;
  }

  /* dump the block info for debug use, only SingleBlockType has label.*/
  void CFGStructurizer::printOrderedBlocks()
  {
    size_t i = 0;
    std::cout << "\n ordered Blocks ->  BasicBlocks -> Current BB: "<< *orderIter << std::endl;
    for (auto iterBlk = orderedBlks.begin(), iterBlkEnd = orderedBlks.end(); iterBlk != iterBlkEnd; ++iterBlk, ++i) {
      std::cout << "B:" << *iterBlk << " BB:" << bTobbmap[*iterBlk];
      if((*iterBlk)->type() == SingleBlockType)
        std::cout << " L:"<< bTobbmap[*iterBlk]->getLabelIndex() << std::endl;
      else
        outBlockTypes((*iterBlk)->type());
    }
  }

  /* transfer the predecessors and successors from the matched blocks to new mergedBB.
   * if the blocks contains backage, should add a successor to itself to make a self loop.*/
  void CFGStructurizer::cfgUpdate(Block* mergedBB,  const BlockSets& blockBBs)
  {
    for(auto iter= blockBBs.begin(); iter != blockBBs.end(); iter++)
    {
      for(auto p = (*iter)->pred_begin(); p != (*iter)->pred_end(); p++)
      {
        if(blockBBs.find(*p) != blockBBs.end())
          continue;

        (*p)->successors().erase(*iter);
        (*p)->successors().insert(mergedBB);
        mergedBB->predecessors().insert(*p);

        if((*p)->fallthrough() == *iter)
          (*p)->fallthrough() = mergedBB;
      }
      for(auto s = (*iter)->succ_begin(); s != (*iter)->succ_end(); s++)
      {
        if(blockBBs.find(*s) != blockBBs.end())
          continue;

        (*s)->predecessors().erase(*iter);
        (*s)->predecessors().insert(mergedBB);
        mergedBB->successors().insert(*s);

        if((*iter)->fallthrough() == *s)
          mergedBB->fallthrough() = *s;
      }
    }

    if(mergedBB->type() != SelfLoopType) {
      for(auto iter= blockBBs.begin(); iter != blockBBs.end(); iter++)
      {
        for(auto s = (*iter)->succ_begin(); s != (*iter)->succ_end(); s++)
        {
          if(blockBBs.find(*s) == blockBBs.end())
            continue;

          LabelIndex l_iter = (*iter)->getEntry()->getLabelIndex();
          LabelIndex l_succ = (*s)->getEntry()->getLabelIndex();
          if(l_iter > l_succ)
          {
            mergedBB->predecessors().insert(mergedBB);
            mergedBB->successors().insert(mergedBB);
            return;
          }
        }
      }
    }
  }

  /* delete the matched blocks and replace it with mergedBB to reduce the CFG.
   * the mergedBB should be inserted to the entry block position. */
  void CFGStructurizer::replace(Block* mergedBB,  BlockSets blockBBs)
  {
    lIterator iter, iterRep;
    bool flag = false;
    for(iter = orderedBlks.begin(); iter!= orderedBlks.end() && !blockBBs.empty();)
    {
      if(!blockBBs.erase(*iter))
      {
        iter++;
        continue;
      }
      if(flag == false)
      {
        iter = orderedBlks.erase(iter);
        iterRep = iter;
        orderIter = orderedBlks.insert(iterRep, mergedBB);
        flag = true;
      }else
      {
        iter = orderedBlks.erase(iter);
      }
    }
  }

  Block* CFGStructurizer::mergeSerialBlock(BlockList& serialBBs)
  {
      Block* p = new SerialBlock(serialBBs);
      BlockList::iterator iter = serialBBs.begin();
      while(iter != serialBBs.end())
      {
        if((*iter)->canBeHandled == false)
        {
          p->canBeHandled = false;
          break;
        }
        p->insnNum += (*iter)->insnNum;
        iter++;
      }
      return insertBlock(p);
  }

  BVAR(OCL_OUTPUT_STRUCTURIZE, false);

  /* if the block has only one successor, and it's successor has only one predecessor 
   * and one successor. the block and the childBlk could be merged to a serial Block.*/
  int CFGStructurizer::serialPatternMatch(Block *block) {
    if (block->succ_size() != 1)
      return 0;

    if(block->hasBarrier())
      return 0;

    Block *childBlk = *block->succ_begin();
    //FIXME, As our barrier implementation doen't support structured barrier
    //operation, exclude all the barrier blocks from serialPatternMatch.
    if (childBlk->pred_size() != 1 || childBlk->hasBarrier() )
      return 0;

    BlockList serialBBs;//childBBs
    BlockSets serialSets;
    serialBBs.push_back(block);
    serialBBs.push_back(childBlk);
    serialSets.insert(block);
    serialSets.insert(childBlk);

    Block* mergedBB = mergeSerialBlock(serialBBs);
    if(mergedBB == NULL)
      return 0;

    cfgUpdate(mergedBB, serialSets);
    replace(mergedBB, serialSets);

    if(OCL_OUTPUT_STRUCTURIZE)
      printOrderedBlocks();
    ++numSerialPatternMatch;
    if(serialSets.find(blocks_entry) != serialSets.end())
      blocks_entry = mergedBB;
    return 1;
  }

  Block* CFGStructurizer::mergeLoopBlock(BlockList& loopSets)
  {
    if(loopSets.size() == 1)
    {
      Block* p = new SelfLoopBlock(*loopSets.begin());
      p->insnNum = (*loopSets.begin())->insnNum;
      p->canBeHandled = true;
      (*loopSets.begin())->getExit()->isLoopExit = true;
      return insertBlock(p);
    }
    return NULL;
  }

  /*match the selfLoop pattern with llvm info or check whether the compacted node has a backage to itself.*/
  int CFGStructurizer::loopPatternMatch(Block *block) {
    Block* loop_header = NULL;
    Block* b = block;
    BlockSets loopSets;
    BlockList loopBBs;

    //if b is basic block , query the llvm loop info to find the loop whoose loop header is b;
    if(block->type() == SingleBlockType){
      for (auto l : loops) {
        BasicBlock &a = fn->getBlock(l->bbs[0]);
        loop_header = bbmap.find(&a)->second;

        if(loop_header == b){
          for (auto bb : l->bbs) {
            BasicBlock &tmp = fn->getBlock(bb);
            Block* block_ = bbmap.find(&tmp)->second;
            loopBBs.push_front(block_);
            loopSets.insert(block_);
          }
          break;
        }
      }
    }else{
      //b is compacted node, it would have a successor pointed to itself for self loop.
      if(block->successors().find(b) != block->successors().end())
      {
        loopBBs.push_front(b);
        loopSets.insert(b);
      }
    }

    if(loopBBs.empty())
      return 0;

    if(loopSets.size() == 1) {
    //self loop header should have a successor to itself, check this before merged.
      Block* lblock = *loopSets.begin();
      if(lblock->successors().find(lblock) == lblock->successors().end())
        return 0;
    }

    Block* mergedBB = mergeLoopBlock(loopBBs);
    if(mergedBB == NULL)
      return 0;

    cfgUpdate(mergedBB, loopSets);
    replace(mergedBB, loopSets);

    if(OCL_OUTPUT_STRUCTURIZE)
      printOrderedBlocks();
    ++numLoopPatternMatch;
    if(loopSets.find(blocks_entry) != loopSets.end())
      blocks_entry = mergedBB;
    return 1;
  }

  /* match the if pattern(E: entry block; T: True block; F: False block; C: Converged block):
  *  for if-else pattern:
  **   E
  **  / \
  ** T   F
  **  \ /
  **   C
  ** E has two edges T and F, T and F both have only one predecessor and one successor indepedently,
  ** the successor of T and F must be the same. E's fallthrough need be treated as True edge.
  *
  *  for if-then pattern E-T-C:
  **   E
  **  / |
  ** T  |
  **  \ |
  **   C
  ** E has two edges T and C,  T should have only one predecessor and one successor, the successor
  ** of T must be C. if E's fallthrough is C, need inverse the predicate.
  *
  *  for if-then pattern E-F-C:
  **   E
  **  | \
  **  |  F
  **  | /
  **   C
  ** E has two edges C and F,  F should have only one predecessor and one successor, the successor
  ** of F must be C. if E's fallthrough is C, need inverse the predicate.
  */
  int CFGStructurizer::ifPatternMatch(Block *block)
  {
    //two edges
    if (block->succ_size() != 2)
      return 0;

    if(block->hasBarrier())
      return 0;

    int NumMatch = 0;
    Block *TrueBB = *block->succ_begin();
    Block *FalseBB = *(++block->succ_begin());
    Block *mergedBB = NULL;
    BlockSets ifSets;

    assert (!TrueBB->succ_empty() || !FalseBB->succ_empty());
    if (TrueBB->succ_size() == 1 && FalseBB->succ_size() == 1
        && TrueBB->pred_size() == 1 && FalseBB->pred_size() == 1
        && *TrueBB->succ_begin() == *FalseBB->succ_begin()
        && !TrueBB->hasBarrier() && !FalseBB->hasBarrier()
        && TrueBB->insnNum < 1000 && FalseBB->insnNum < 1000) {
      // if-else pattern
      ifSets.insert(block);
      if(block->fallthrough() == TrueBB) {
        ifSets.insert(TrueBB);
        ifSets.insert(FalseBB);
        mergedBB = new IfElseBlock(block, TrueBB, FalseBB);
      }else if(block->fallthrough() == FalseBB) {
        ifSets.insert(FalseBB);
        ifSets.insert(TrueBB);
        mergedBB = new IfElseBlock(block, FalseBB, TrueBB);
      }else{
        GBE_ASSERT(0);
      }
      mergedBB->insnNum = block->insnNum + TrueBB->insnNum + FalseBB->insnNum;

      if(block->canBeHandled == false || TrueBB->canBeHandled == false || FalseBB->canBeHandled == false)
        block->canBeHandled = false;

      insertBlock(mergedBB);
    } else if (TrueBB->succ_size() == 1 && TrueBB->pred_size() == 1 &&
        *TrueBB->succ_begin() == FalseBB && !TrueBB->hasBarrier() && TrueBB->insnNum < 1000 ) {
      // if-then pattern, false is empty
      ifSets.insert(block);
      ifSets.insert(TrueBB);
      mergedBB = new IfThenBlock(block, TrueBB);
      mergedBB->insnNum = block->insnNum + TrueBB->insnNum;
      if(block->fallthrough() == FalseBB)
        block->inversePredicate = false;

      if(block->canBeHandled == false || TrueBB->canBeHandled == false)
        block->canBeHandled = false;

      insertBlock(mergedBB);
    } else if (FalseBB->succ_size() == 1 && FalseBB->pred_size() == 1 &&
        *FalseBB->succ_begin() == TrueBB && !FalseBB->hasBarrier() && FalseBB->insnNum < 1000 ) {
      // if-then pattern, true is empty
      ifSets.insert(block);
      ifSets.insert(FalseBB);
      mergedBB = new IfThenBlock(block, FalseBB);
      mergedBB->insnNum = block->insnNum + FalseBB->insnNum;
      if(block->fallthrough() == TrueBB)
        block->inversePredicate = false;

      if(block->canBeHandled == false || FalseBB->canBeHandled == false)
        block->canBeHandled = false;

      insertBlock(mergedBB);
    }
    else{
      return 0;
    }

    if(ifSets.empty())
      return 0;

    if(mergedBB == NULL)
      return 0;

    cfgUpdate(mergedBB, ifSets);
    replace(mergedBB, ifSets);

    if(OCL_OUTPUT_STRUCTURIZE)
      printOrderedBlocks();
    ++numIfPatternMatch;
    if(ifSets.find(blocks_entry) != ifSets.end())
      blocks_entry = mergedBB;
    return NumMatch + 1;
  }

  /* match loop pattern, serail pattern, if pattern accordingly, update and replace block the CFG internally once matched. */
  int CFGStructurizer::patternMatch(Block *block) {
    int NumMatch = 0;
    NumMatch += loopPatternMatch(block);
    NumMatch += serialPatternMatch(block);
    NumMatch += ifPatternMatch(block);
    return NumMatch;
  }

  void CFGStructurizer::blockPatternMatch()
  {
    int increased = 0;

    do
    {
      increased = numSerialPatternMatch + numLoopPatternMatch + numIfPatternMatch;

      orderIter = orderedBlks.begin();

      while(orderedBlks.size() > 1 && orderIter != orderedBlks.end())
      {
        if(OCL_OUTPUT_STRUCTURIZE)
          printOrderedBlocks();
        patternMatch(*orderIter);
        orderIter++;
      }
      if(OCL_OUTPUT_STRUCTURIZE)
        printOrderedBlocks();

      if(increased == numSerialPatternMatch + numLoopPatternMatch + numIfPatternMatch)
        break;

    } while(orderedBlks.size()>1);
    if(OCL_OUTPUT_STRUCTURIZE)
      std::cout << "Serial:" << numSerialPatternMatch << "Loop:" << numLoopPatternMatch << "If:" << numIfPatternMatch << std::endl;
  }

  void CFGStructurizer::StructurizeBlocks()
  {
    initializeBlocks();
    blockPatternMatch();
    handleStructuredBlocks();
    calculateNecessaryLiveout();
  }
} /* namespace ir */
} /* namespace gbe */
