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
#ifndef __STRUCTURIZER_HPP__
#define __STRUCTURIZER_HPP__
#include "llvm/ADT/SmallVector.h"
#include "ir/unit.hpp"
#include "ir/function.hpp"
#include "ir/instruction.hpp"

#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
namespace gbe {
namespace ir {
  using namespace llvm;

  enum BlockType
  {
    SingleBlockType = 0,
    SerialBlockType,
    IfThenType,
    IfElseType,
    SelfLoopType
  };

  /* Block*/
  class Block;

  typedef std::set<Block *> BlockSets;
  typedef std::list<Block *> BlockList;
  typedef std::vector<Block *> BlockVector;
  typedef std::set<Block *>::iterator sIterator;
  typedef std::list<Block *>::iterator lIterator;

  class Block
  {
  public:
    Block(BlockType type, const BlockList& children): has_barrier(false), mark(false), canBeHandled(true), inversePredicate(true), insnNum(0)
    {
      this->btype = type;
      this->children = children;
    }
    virtual ~Block() {}
    Block*& fallthrough() { return fall_through; }
    BlockSets& successors() { return successor; }
    size_t succ_size() { return successor.size(); }
    sIterator succ_begin() { return successor.begin(); }
    sIterator succ_end() { return successor.end(); }
    bool succ_empty() { return successor.empty(); }
    BlockSets& predecessors() { return predecessor; }
    size_t pred_size() { return predecessor.size(); }
    sIterator pred_begin() { return predecessor.begin(); }
    sIterator pred_end() { return predecessor.end(); }
    bool& hasBarrier() { return has_barrier; }
    BlockType type() { return btype; }
    virtual BasicBlock* getEntry()
    {
      return (*(children.begin()))->getEntry();
    }
    virtual BasicBlock* getExit()
    {
      return (*(children.rbegin()))->getExit();
    }

  public:
    BlockType btype;
    Block* fall_through;
    BlockSets predecessor;
    BlockSets successor;
    BlockList children;
    bool has_barrier;
    bool mark;
    bool canBeHandled;
    //label is for debug
    int label;
    /* inversePredicate should be false under two circumstance,
     * fallthrough is the same with succs:
     * (1) n->succs == m && block->fallthrough == m
     * block
     * | \
     * |  \
     * m<--n
     * (2) m->succs == n && block->fallthrough == n
     * block
     * | \
     * |  \
     * m-->n
     * */
    bool inversePredicate;
    int insnNum;
  };

  /* represents basic block */
  class SimpleBlock: public Block
  {
  public:
    SimpleBlock(BasicBlock *p_bb) : Block(SingleBlockType, BlockList()) { this->p_bb = p_bb; }
    virtual ~SimpleBlock() {}
    BasicBlock* getBasicBlock() { return p_bb; }
    virtual BasicBlock* getEntry() { return p_bb; }
    virtual BasicBlock* getExit() { return p_bb; }
    virtual BasicBlock* getFirstBB() { return p_bb; }
  private:
    BasicBlock *p_bb;
  };

  /* a serial of Blocks*/
  class SerialBlock : public Block
  {
  public:
    SerialBlock(BlockList& children) : Block(SerialBlockType, children) {}
    virtual ~SerialBlock(){}
  };

  /* If-Then Block*/
  class IfThenBlock : public Block
  {
  public:
    IfThenBlock(Block* pred, Block* trueBlock) : Block(IfThenType, InitChildren(pred, trueBlock)) {}
    virtual ~IfThenBlock() {}

  private:
    const BlockList InitChildren(Block* pred, Block* trueBlock)
    {
      BlockList children;
      children.push_back(pred);
      children.push_back(trueBlock);
      return children;
    }
  };

  /* If-Else Block*/
  class IfElseBlock: public Block
  {
  public:
    IfElseBlock(Block* pred, Block* trueBlock, Block* falseBlock) : Block(IfElseType, InitChildren(pred, trueBlock, falseBlock)) {}
    virtual ~IfElseBlock() {}

  private:
    const BlockList InitChildren(Block* pred, Block* trueBlock, Block* falseBlock)
    {
      BlockList children;
      children.push_back(pred);
      children.push_back(trueBlock);
      children.push_back(falseBlock);
      return children;
    }
  };

  /* Self loop Block*/
  class SelfLoopBlock: public Block
  {
  public:
    SelfLoopBlock(Block* block) : Block(SelfLoopType, InitChildren(block)) {}
    virtual ~SelfLoopBlock() {}
    virtual BasicBlock* getEntry()
    {
      return (*(children.begin()))->getEntry();
    }
    virtual BasicBlock* getExit()
    {
      return (*(children.begin()))->getExit();
    }

  private:
    const BlockList InitChildren(Block * block)
    {
      BlockList children;
      children.push_back(block);
      return children;
    }
  };

  class CFGStructurizer{
    public:
      CFGStructurizer(Function* fn) { this->fn = fn; numSerialPatternMatch = 0; numLoopPatternMatch = 0; numIfPatternMatch = 0;}
      ~CFGStructurizer();

      void StructurizeBlocks();

    private:
      int  numSerialPatternMatch;
      int  numLoopPatternMatch;
      int  numIfPatternMatch;

      void outBlockTypes(BlockType type);
      void printOrderedBlocks();
      void blockPatternMatch();
      int  serialPatternMatch(Block *block);
      Block* mergeSerialBlock(BlockList& serialBB);
      void cfgUpdate(Block* mergedBB,  const BlockSets& blockBBs);
      void replace(Block* mergedBB,  BlockSets serialSets);
      int  loopPatternMatch(Block *block);
      Block* mergeLoopBlock(BlockList& loopSets);
      int  ifPatternMatch(Block *block);
      int  patternMatch(Block *block);
      void collectInsnNum(Block* block, const BasicBlock* bb);

    private:
      void handleSelfLoopBlock(Block *loopblock, LabelIndex& whileLabel);
      void markNeedIf(Block *block, bool status);
      void markNeedEndif(Block *block, bool status);
      void markStructuredBlocks(Block *block, bool status);
      void handleIfBlock(Block *block, LabelIndex& matchingEndifLabel, LabelIndex& matchingElseLabel);
      void handleThenBlock(Block * block, LabelIndex& endiflabel);
      void handleThenBlock2(Block *block, Block *elseblock, LabelIndex elseBBLabel);
      void handleElseBlock(Block * block, LabelIndex& elselabel, LabelIndex& endiflabel);
      void handleStructuredBlocks();
      void getStructureSequence(Block *block, std::vector<BasicBlock*> &seq);
      std::set<int> getStructureBasicBlocksIndex(Block* block, std::vector<BasicBlock *> &bbs);
      std::set<BasicBlock *> getStructureBasicBlocks(Block *block);
      Block* insertBlock(Block *p_block);
      bool checkForBarrier(const BasicBlock* bb);
      void getLiveIn(BasicBlock& bb, std::set<Register>& livein);
      void initializeBlocks();
      void calculateNecessaryLiveout();

    private:
      Function *fn;
      std::map<BasicBlock *, Block *> bbmap;
      std::map<Block *, BasicBlock *> bTobbmap;
      BlockVector blocks;
      Block* blocks_entry;
      gbe::vector<Loop *> loops;
      BlockList orderedBlks;
      BlockList::iterator orderIter;
  };
} /* namespace ir */
} /* namespace gbe */

#endif
